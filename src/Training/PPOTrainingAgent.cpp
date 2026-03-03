/**
 * @file PPOTrainingAgent.cpp
 * @brief Implementation of the PPOTrainingAgent.
 */

#include "PPOTrainingAgent.hpp"
#include <algorithm>
#include <iostream>

#include "Simulation/Room/Room.hpp"

using namespace POLA::Training;
using namespace POLA::Common;
using namespace POLA::Interfaces;

PPOTrainingAgent::PPOTrainingAgent(const forge::ProviderRef &provider,
                                   const TrainingConfig &config, uint32_t seed)
    : _provider(provider), _config(config), _rewardFn(config) {
  if (torch::cuda::is_available()) {
    _device = torch::kCUDA;
    std::cout << "[PPO] Using CUDA device" << std::endl;
  } else if (torch::mps::is_available()) {
    _device = torch::kMPS;
    std::cout << "[PPO] Using MPS device" << std::endl;
  } else {
    _device = torch::kCPU;
    std::cout << "[PPO] Using CPU device" << std::endl;
  }

  _actorCritic =
      ActorCritic(config.stateDim, config.actionDim, config.hiddenDim);
  _actorCritic->to(_device);

  _optimizer = std::make_unique<torch::optim::Adam>(
      _actorCritic->parameters(),
      torch::optim::AdamOptions(config.learningRate));

  _rollout.reserve(_config.rolloutSteps);
}

torch::Tensor PPOTrainingAgent::normalizeState(const AIState &state) const {
  auto tensor = torch::tensor(
      {static_cast<float>(StateNorm::normalize(
           state.tempIn, StateNorm::tempIn_offset, StateNorm::tempIn_scale)),
       static_cast<float>(StateNorm::normalize(
           state.tempOut, StateNorm::tempOut_offset, StateNorm::tempOut_scale)),
       static_cast<float>(StateNorm::normalize(state.electricityPrice,
                                               StateNorm::price_offset,
                                               StateNorm::price_scale)),
       static_cast<float>(StateNorm::normalize(
           state.gpsDistance, StateNorm::dist_offset, StateNorm::dist_scale)),
       static_cast<float>(StateNorm::normalize(
           state.userVelocity, StateNorm::vel_offset, StateNorm::vel_scale)),
       static_cast<float>(StateNorm::normalize(state.targetTemp,
                                               StateNorm::target_offset,
                                               StateNorm::target_scale))},
      torch::TensorOptions().dtype(torch::kFloat32).device(_device));

  return tensor.unsqueeze(0); // [1, 6]
}

double PPOTrainingAgent::predict(const AIState &currentState) {
  const auto stateTensor = normalizeState(currentState);

  // 1. Process the previous step's reward (if any)
  if (_prevState.has_value() && _prevTransition.has_value()) {
    const double powerW =
        _prevTransition->actionLogit; // NOTE: Action here was sigmoid applied

    // At this point in the simulator code, a step has occurred.
    // We know the old state, the old action, and the resulting new state.
    double reward = _rewardFn.compute(_prevState.value(), powerW, currentState);

    // Store the transition
    _prevTransition->reward = static_cast<float>(reward);
    _rollout.push_back(_prevTransition.value());

    // Check if rollout buffer is full
    if (_rollout.size() == _config.rolloutSteps) {
      _rollout.back().done = true; // Mark episode as done
      updatePPO(stateTensor);      // Train the network!

      auto room = _provider.get<Simulation::Room>();
      room->setTemperature(
          20.0); // Reset room temp to a default value (or randomize)

      const double avgReward =
          std::accumulate(_rollout.begin(), _rollout.end(), 0.0,
                          [](double sum, const RolloutTransition &t) {
                            return sum + t.reward;
                          }) /
          _config.rolloutSteps;

      std::cout << "[PPO] Rollout " << _numRollouts
                << " | Steps: " << _totalSteps << "/" << _config.totalTimesteps
                << " | Avg Reward: " << avgReward << std::endl;

      _rollout.clear();

      // Randomize environment for next episode!
      // if (_envControl)
      // {
      // TODO: Implement environment randomization logic here (e.g., randomize
      // starting room temp) _envControl->resetEnvironment();
      // }
    }
  }

  // 2. Select the next action
  _actorCritic->eval(); // Inference mode
  auto [actionLogit, logProb, value] = _actorCritic->act(stateTensor);

  double rawLogit = actionLogit.item<float>();
  double actionSigmoid = torch::sigmoid(actionLogit).item<double>();
  double powerW = std::clamp(actionSigmoid, 0.0, 1.0);

  // 3. Save state for next tick's reward calculation
  _prevState = currentState;

  RolloutTransition transition;
  transition.stateTensor = stateTensor.squeeze(0);
  transition.actionLogit = rawLogit;
  transition.logProb = logProb.item<float>();
  transition.value = value.item<float>();
  transition.reward = 0.0f; // Calculated next tick
  transition.done = false;

  _prevTransition = transition;
  _totalSteps++;

  return powerW;
}

void PPOTrainingAgent::updatePPO(const torch::Tensor &finalStateTensor) {
  _actorCritic->train(); // Training mode

  // Bootstrap: estimate value of the final state
  auto [_, __, lastValueTensor] = _actorCritic->act(finalStateTensor);
  const float lastValue = lastValueTensor.item<float>();

  const int N = _rollout.size();

  // ---- Compute GAE ----
  std::vector<float> advantages(N, 0.0f);
  std::vector<float> returns(N, 0.0f);
  float lastGAE = 0.0f;

  for (int t = N - 1; t >= 0; --t) {
    const float nextValue = (t == N - 1) ? lastValue : _rollout[t + 1].value;
    const float mask = _rollout[t].done ? 0.0f : 1.0f;
    const float delta = _rollout[t].reward +
                        static_cast<float>(_config.gamma) * nextValue * mask -
                        _rollout[t].value;

    lastGAE = delta + static_cast<float>(_config.gamma * _config.lambda) *
                          mask * lastGAE;
    advantages[t] = lastGAE;
    returns[t] = advantages[t] + _rollout[t].value;
  }

  // Convert to tensors
  std::vector<torch::Tensor> stateTensors;
  std::vector<float> actionLogits, logProbs, rewards;

  for (const auto &r : _rollout) {
    stateTensors.push_back(r.stateTensor);
    actionLogits.push_back(r.actionLogit);
    logProbs.push_back(r.logProb);
    rewards.push_back(r.reward);
  }

  auto statesTensor = torch::stack(stateTensors).to(_device);
  auto actionLogitsTensor =
      torch::tensor(
          actionLogits,
          torch::TensorOptions().dtype(torch::kFloat32).device(_device))
          .unsqueeze(1);
  auto oldLogProbsTensor = torch::tensor(
      logProbs, torch::TensorOptions().dtype(torch::kFloat32).device(_device));
  auto advantagesTensor = torch::tensor(
      advantages,
      torch::TensorOptions().dtype(torch::kFloat32).device(_device));
  auto returnsTensor = torch::tensor(
      returns, torch::TensorOptions().dtype(torch::kFloat32).device(_device));

  advantagesTensor = (advantagesTensor - advantagesTensor.mean()) /
                     (advantagesTensor.std() + 1e-8);

  // ---- PPO Clipped Update ----
  for (int epoch = 0; epoch < _config.numEpochs; ++epoch) {
    auto indices = torch::randperm(
        N, torch::TensorOptions().dtype(torch::kLong).device(_device));

    for (int start = 0; start < N; start += _config.miniBatchSize) {
      const int end = std::min(start + _config.miniBatchSize, N);
      auto mbIdx = indices.slice(0, start, end);

      auto mbStates = statesTensor.index_select(0, mbIdx);
      auto mbActionLogits = actionLogitsTensor.index_select(0, mbIdx);
      auto mbOldLogProbs = oldLogProbsTensor.index_select(0, mbIdx);
      auto mbAdvantages = advantagesTensor.index_select(0, mbIdx);
      auto mbReturns = returnsTensor.index_select(0, mbIdx);

      auto [newLogProbs, newValues, entropy] =
          _actorCritic->evaluate(mbStates, mbActionLogits);

      auto ratio = torch::exp(newLogProbs - mbOldLogProbs);
      auto surr1 = ratio * mbAdvantages;
      auto surr2 = torch::clamp(ratio, 1.0 - _config.clipEpsilon,
                                1.0 + _config.clipEpsilon) *
                   mbAdvantages;
      auto policyLoss = -torch::min(surr1, surr2).mean();

      auto valueLoss = torch::nn::functional::mse_loss(newValues, mbReturns);
      auto entropyLoss = -entropy.mean();

      auto totalLoss = policyLoss + _config.valueCoeff * valueLoss +
                       _config.entropyCoeff * entropyLoss;

      _optimizer->zero_grad();
      totalLoss.backward();
      torch::nn::utils::clip_grad_norm_(_actorCritic->parameters(),
                                        _config.maxGradNorm);
      _optimizer->step();
    }
  }

  _numRollouts++;

  // Logging
  if (_numRollouts % _config.logInterval == 0) {
    double totalReward = 0.0;
    for (float r : rewards)
      totalReward += r;
    const double avgReward = totalReward / N;

    std::cout << "[PPO] Rollout " << _numRollouts << " | Steps: " << _totalSteps
              << " | Avg Reward: " << avgReward << std::endl;

    if (avgReward > _bestAvgReward) {
      _bestAvgReward = avgReward;
    }
  }

  // Save Checkpoint
  if (_numRollouts % _config.saveInterval == 0) {
    exportModel();
    std::cout << "[PPO] Checkpoint saved to: " << _config.modelSavePath
              << std::endl;
  }
}

void PPOTrainingAgent::exportModel() {
  _actorCritic->exportActor(_config.modelSavePath);
}
