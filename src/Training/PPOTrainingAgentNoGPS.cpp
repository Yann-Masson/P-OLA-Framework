/**
 * @file PPOTrainingAgentNoGPS.cpp
 * @brief Implementation of the GPS-free PPO training agent.
 *
 * State vector layout (40 dimensions, matching ActorCriticNoGPS.exportActor):
 *   [0]     tempIn
 *   [1]     electricityPrice
 *   [2..13] weather.forecast[0..5] (outdoorTemp, sunlightLux per hour × 6)
 *   [14]    userPreferences.minTemperature
 *   [15]    userPreferences.maxTemperature
 *   [16..39] userSchedule.userPresent[0..23]
 */

#include "PPOTrainingAgentNoGPS.hpp"
#include <algorithm>
#include <filesystem>
#include <iostream>

#include "Common/DataTypes.hpp"
#include "Interfaces/IAIRecorder.hpp"
#include "Interfaces/IClock.hpp"
#include "Interfaces/IInputService.hpp"
#include "Simulation/Room/Room.hpp"

using namespace POLA::Training;
using namespace POLA::Common;
using namespace POLA::Interfaces;

// No-GPS state is 40 dims: tempIn(1) + price(1) + weather(12) + prefs(2) +
// schedule(24)
static constexpr int kNoGPSStateDim = 40;

PPOTrainingAgentNoGPS::PPOTrainingAgentNoGPS(const forge::ProviderRef &provider,
                                             const TrainingConfig &config,
                                             uint32_t /*seed*/)
    : _provider(provider), _config(config), _rewardFn(provider, config) {
  // Force stateDim to 40 regardless of what was passed in
  _config.stateDim = kNoGPSStateDim;

  if (torch::cuda::is_available()) {
    _device = torch::kCUDA;
    std::cout << "[PPO-NoGPS] Using CUDA device" << std::endl;
  } else if (torch::mps::is_available()) {
    _device = torch::kMPS;
    std::cout << "[PPO-NoGPS] Using MPS device" << std::endl;
  } else {
    _device = torch::kCPU;
    std::cout << "[PPO-NoGPS] Using CPU device" << std::endl;
  }

  _actorCritic =
      ActorCriticNoGPS(kNoGPSStateDim, _config.actionDim, _config.hiddenDim);
  _actorCritic->to(_device);

  _optimizer = std::make_unique<torch::optim::Adam>(
      _actorCritic->parameters(),
      torch::optim::AdamOptions(_config.learningRate));

  _rollout.reserve(_config.rolloutSteps);

  // Resume from checkpoint if one exists
  const std::string ckptPath = _config.modelSavePath + ".ckpt";
  if (std::filesystem::exists(ckptPath)) {
    try {
      _actorCritic->loadCheckpoint(ckptPath);
      std::cout << "[PPO-NoGPS] Resumed from checkpoint: " << ckptPath
                << std::endl;
    } catch (const std::exception &e) {
      std::cerr << "[PPO-NoGPS] Warning: failed to load checkpoint '"
                << ckptPath << "': " << e.what() << " — starting from scratch."
                << std::endl;
    }
  } else {
    std::cout << "[PPO-NoGPS] No checkpoint at '" << ckptPath
              << "' — starting from scratch." << std::endl;
  }
}

torch::Tensor
PPOTrainingAgentNoGPS::normalizeState(const AIStateNoGPS &state) const {
  // Flatten to 40 floats matching ActorCriticNoGPS column layout
  std::vector<float> values;
  values.reserve(kNoGPSStateDim);

  // [0] tempIn
  values.push_back(static_cast<float>(StateNorm::normalize(
      state.tempIn, StateNorm::tempIn_offset, StateNorm::tempIn_scale)));
  // [1] electricityPrice
  values.push_back(static_cast<float>(
      StateNorm::normalize(state.electricityPrice, StateNorm::price_offset,
                           StateNorm::price_scale)));

  // [2..13] weather forecast (6 hours × 2 features)
  for (const auto &wp : state.weather.forecast) {
    values.push_back(static_cast<float>(
        StateNorm::normalize(wp.outdoorTemp, StateNorm::forecastTemp_offset,
                             StateNorm::forecastTemp_scale)));
    values.push_back(static_cast<float>(StateNorm::normalize(
        wp.sunlightLuxIntensity, StateNorm::sunlight_offset,
        StateNorm::sunlight_scale)));
  }

  // [14] userPreferences.minTemperature
  values.push_back(static_cast<float>(StateNorm::normalize(
      state.userPreferences.minTemperature, StateNorm::prefTemp_offset,
      StateNorm::prefTemp_scale)));
  // [15] userPreferences.maxTemperature
  values.push_back(static_cast<float>(StateNorm::normalize(
      state.userPreferences.maxTemperature, StateNorm::prefTemp_offset,
      StateNorm::prefTemp_scale)));

  // [16..39] userSchedule (24 hours, binary)
  for (const bool present : state.userSchedule.userPresent)
    values.push_back(present ? 1.0f : 0.0f);

  auto tensor = torch::tensor(
      values, torch::TensorOptions().dtype(torch::kFloat32).device(_device));

  return tensor.unsqueeze(0); // [1, 40]
}

double PPOTrainingAgentNoGPS::predict(const AIStateNoGPS &currentState) {
  const auto stateTensor = normalizeState(currentState);

  // 1. Reward the previous transition
  if (_prevState.has_value() && _prevTransition.has_value()) {
    const float logit = _prevTransition->actionLogit;
    const double powerW = std::clamp(1.0 / (1.0 + std::exp(-logit)), 0.0, 1.0);

    const double reward =
        _rewardFn.compute(_prevState.value(), powerW, currentState);

    const double timestamp =
        _provider.get<IClock>()->getElapsedTimeSinceStart();
    const double actualPower = std::clamp(powerW, 0.0, 1.0);

    // IAIRecorder still expects the full AIState — build a compatible one
    // by zeroing GPS fields so the recorder CSV remains well-formed.
    AIState recordState{};
    recordState.tempIn = currentState.tempIn;
    recordState.electricityPrice = currentState.electricityPrice;
    recordState.userDistanceKm = 0.0; // n/a in no-GPS model
    recordState.userVelocityKmMin = 0.0;
    recordState.weather = currentState.weather;
    recordState.userPreferences = currentState.userPreferences;
    recordState.userSchedule = currentState.userSchedule;

    const auto recorder = _provider.get<IAIRecorder>();
    recorder->record(timestamp, recordState, powerW, actualPower, reward);

    _prevTransition->reward = static_cast<float>(reward);
    _rollout.push_back(_prevTransition.value());

    if (_rollout.size() == static_cast<size_t>(_config.rolloutSteps)) {
      _rollout.back().done = true;
      updatePPO(stateTensor);
      _rollout.clear();

      const auto room = _provider.get<Simulation::Room>();
      const auto userPrefService =
          _provider.get<IInputService<UserPreferenceData>>();
      const auto userPref = userPrefService->getInput();
      room->reset((userPref.minTemperature + userPref.maxTemperature) / 2.0);
    }
  }

  // 2. Sample action from current policy
  _actorCritic->eval();
  auto [actionLogit, logProb, value] = _actorCritic->act(stateTensor);

  const double rawLogit = actionLogit.item<float>();
  const double action = torch::sigmoid(actionLogit).item<double>();
  const double powerW = std::clamp(action, 0.0, 1.0);

  // 3. Store transition
  _prevState = currentState;

  RolloutTransition transition;
  transition.stateTensor = stateTensor.squeeze(0);
  transition.actionLogit = static_cast<float>(rawLogit);
  transition.logProb = logProb.item<float>();
  transition.value = value.item<float>();
  transition.reward = 0.0f;
  transition.done = false;

  _prevTransition = transition;
  _totalSteps++;

  return powerW;
}

void PPOTrainingAgentNoGPS::updatePPO(const torch::Tensor &finalStateTensor) {
  _actorCritic->train();

  auto [_, __, lastValueTensor] = _actorCritic->act(finalStateTensor);
  const float lastValue = lastValueTensor.item<float>();

  const int N = static_cast<int>(_rollout.size());

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

  std::vector<torch::Tensor> stateTensors;
  std::vector<float> actionLogits, logProbs, rewards;

  for (const auto &r : _rollout) {
    stateTensors.push_back(r.stateTensor);
    actionLogits.push_back(r.actionLogit);
    logProbs.push_back(r.logProb);
    rewards.push_back(r.reward);
  }

  auto statesTensor = torch::stack(stateTensors).to(_device); // [N, 40]
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

  if (_numRollouts % _config.logInterval == 0) {
    double totalReward = 0.0;
    for (float r : rewards)
      totalReward += r;
    const double avgReward = totalReward / N;

    std::cout << "[PPO-NoGPS] Rollout " << _numRollouts
              << " | Steps: " << _totalSteps << "/" << _config.totalTimesteps
              << " | Avg Reward: " << avgReward << std::endl;

    if (avgReward > _bestAvgReward)
      _bestAvgReward = avgReward;
  }

  if (_numRollouts % _config.saveInterval == 0) {
    exportModel();
    std::cout << "[PPO-NoGPS] Checkpoint saved to: " << _config.modelSavePath
              << std::endl;
  }
}

void PPOTrainingAgentNoGPS::exportModel() {
  _actorCritic->exportActor(_config.modelSavePath);
  _actorCritic->saveCheckpoint(_config.modelSavePath + ".ckpt");
}
