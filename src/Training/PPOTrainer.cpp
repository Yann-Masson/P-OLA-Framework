/**
 * @file PPOTrainer.cpp
 * @brief Implementation of the PPO training loop for the smart thermostat.
 */

#include "PPOTrainer.hpp"
#include "TrainingConfig.hpp"

#include <iostream>
#include <numeric>
#include <algorithm>
#include <filesystem>
#include <cmath>

using namespace POLA::Training;
using namespace POLA::Common;

// ============================================================================
// RolloutData
// ============================================================================

void PPOTrainer::RolloutData::clear()
{
    states.clear();
    actionLogits.clear();
    logProbs.clear();
    values.clear();
    rewards.clear();
    dones.clear();
}

// ============================================================================
// PPOTrainer
// ============================================================================

PPOTrainer::PPOTrainer(
    forge::ProviderRef provider,
    const TrainingConfig &config,
    const uint32_t seed
)
    : _config(config)
    , _env(provider, config, seed)
    , _rewardFn(provider, config)
{
    // Select compute device
    if (torch::cuda::is_available())
    {
        _device = torch::kCUDA;
        std::cout << "[PPO] Using CUDA device" << std::endl;
    }
    else if (torch::mps::is_available())
    {
        _device = torch::kMPS;
        std::cout << "[PPO] Using MPS device" << std::endl;
    }
    else
    {
        _device = torch::kCPU;
        std::cout << "[PPO] Using CPU device" << std::endl;
    }

    // Create actor-critic network
    _actorCritic = ActorCritic(config.stateDim, config.actionDim, config.hiddenDim);
    _actorCritic->to(_device);

    // Adam optimizer
    _optimizer = std::make_unique<torch::optim::Adam>(
        _actorCritic->parameters(),
        torch::optim::AdamOptions(config.learningRate));
}

torch::Tensor PPOTrainer::normalizeState(const AIState &state) const
{
    auto tensor = torch::tensor({static_cast<float>(StateNorm::normalize(state.tempIn, StateNorm::tempIn_offset, StateNorm::tempIn_scale)),
                                 static_cast<float>(StateNorm::normalize(state.tempOut, StateNorm::tempOut_offset, StateNorm::tempOut_scale)),
                                 static_cast<float>(StateNorm::normalize(state.electricityPrice, StateNorm::price_offset, StateNorm::price_scale)),
                                 static_cast<float>(StateNorm::normalize(state.gpsDistance, StateNorm::dist_offset, StateNorm::dist_scale)),
                                 static_cast<float>(StateNorm::normalize(state.userVelocity, StateNorm::vel_offset, StateNorm::vel_scale)),
                                 static_cast<float>(StateNorm::normalize(state.targetTemp, StateNorm::target_offset, StateNorm::target_scale))},
                                torch::TensorOptions().dtype(torch::kFloat32).device(_device));

    return tensor.unsqueeze(0); // [1, 6]
}

// ============================================================================
// Rollout Collection
// ============================================================================

void PPOTrainer::collectRollout()
{
    _rollout.clear();
    _actorCritic->eval();

    AIState currentState = _env.reset();

    for (int step = 0; step < _config.rolloutSteps; ++step)
    {
        auto stateTensor = normalizeState(currentState);

        // Sample action from the current policy
        auto [actionLogit, logProb, value] = _actorCritic->act(stateTensor);

        // Convert logit to action via sigmoid for the environment
        double action = torch::sigmoid(actionLogit).item<double>();
        action = std::clamp(action, 0.0, 1.0);

        // Step the environment
        auto [nextState, reward, done] = _env.step(action);

        // Store transition data
        _rollout.states.push_back(stateTensor.squeeze(0));
        _rollout.actionLogits.push_back(actionLogit.item<float>());
        _rollout.logProbs.push_back(logProb.item<float>());
        _rollout.values.push_back(value.item<float>());
        _rollout.rewards.push_back(static_cast<float>(reward));
        _rollout.dones.push_back(done);

        if (done)
        {
            currentState = _env.reset();
        }
        else
        {
            currentState = nextState;
        }

        _totalSteps++;
    }

    // Bootstrap: estimate value of the final state (needed for GAE)
    auto lastStateTensor = normalizeState(currentState);
    auto [_, __, lastValue] = _actorCritic->act(lastStateTensor);
    _rollout.values.push_back(lastValue.item<float>());
}

// ============================================================================
// PPO Update
// ============================================================================

void PPOTrainer::update()
{
    _actorCritic->train();

    const int N = _config.rolloutSteps;

    // ---- Compute GAE (Generalized Advantage Estimation) ----
    // δ_t = r_t + γ * V(s_{t+1}) * (1 - done) - V(s_t)
    // A_t = Σ_{l=0}^{T-t-1} (γλ)^l * δ_{t+l}
    std::vector<float> advantages(N, 0.0f);
    std::vector<float> returns(N, 0.0f);

    float lastGAE = 0.0f;
    for (int t = N - 1; t >= 0; --t)
    {
        const float nextValue = _rollout.values[t + 1];
        const float mask = _rollout.dones[t] ? 0.0f : 1.0f;
        const float delta = _rollout.rewards[t] + static_cast<float>(_config.gamma) * nextValue * mask - _rollout.values[t];

        lastGAE = delta + static_cast<float>(_config.gamma * _config.lambda) * mask * lastGAE;

        advantages[t] = lastGAE;
        returns[t] = advantages[t] + _rollout.values[t];
    }

    // ---- Convert rollout data to tensors ----
    auto statesTensor = torch::stack(_rollout.states).to(_device); // [N, 6]
    auto actionLogitsTensor = torch::tensor(_rollout.actionLogits,
                                            torch::TensorOptions().dtype(torch::kFloat32).device(_device))
                                  .unsqueeze(1); // [N, 1]
    auto oldLogProbsTensor = torch::tensor(_rollout.logProbs,
                                           torch::TensorOptions().dtype(torch::kFloat32).device(_device)); // [N]
    auto advantagesTensor = torch::tensor(advantages,
                                          torch::TensorOptions().dtype(torch::kFloat32).device(_device)); // [N]
    auto returnsTensor = torch::tensor(returns,
                                       torch::TensorOptions().dtype(torch::kFloat32).device(_device)); // [N]

    // Normalize advantages (standard practice — stabilizes training)
    advantagesTensor = (advantagesTensor - advantagesTensor.mean()) / (advantagesTensor.std() + 1e-8);

    // ---- PPO clipped update for K epochs ----
    for (int epoch = 0; epoch < _config.numEpochs; ++epoch)
    {

        // Random shuffle for mini-batch construction
        auto indices = torch::randperm(N,
                                       torch::TensorOptions().dtype(torch::kLong).device(_device));

        for (int start = 0; start < N; start += _config.miniBatchSize)
        {
            const int end = std::min(start + _config.miniBatchSize, N);
            auto mbIdx = indices.slice(0, start, end);

            // Select mini-batch data
            auto mbStates = statesTensor.index_select(0, mbIdx);
            auto mbActionLogits = actionLogitsTensor.index_select(0, mbIdx);
            auto mbOldLogProbs = oldLogProbsTensor.index_select(0, mbIdx);
            auto mbAdvantages = advantagesTensor.index_select(0, mbIdx);
            auto mbReturns = returnsTensor.index_select(0, mbIdx);

            // Evaluate actions under the CURRENT policy
            auto [newLogProbs, newValues, entropy] =
                _actorCritic->evaluate(mbStates, mbActionLogits);

            // ---- Clipped Surrogate Objective ----
            // ratio = π_new(a|s) / π_old(a|s)
            auto ratio = torch::exp(newLogProbs - mbOldLogProbs);
            auto surr1 = ratio * mbAdvantages;
            auto surr2 = torch::clamp(ratio,
                                      1.0 - _config.clipEpsilon,
                                      1.0 + _config.clipEpsilon) *
                         mbAdvantages;
            auto policyLoss = -torch::min(surr1, surr2).mean();

            // ---- Value Loss (MSE) ----
            auto valueLoss = torch::nn::functional::mse_loss(newValues, mbReturns);

            // ---- Entropy Bonus (encourages exploration) ----
            auto entropyLoss = -entropy.mean();

            // ---- Total Loss ----
            auto totalLoss = policyLoss + _config.valueCoeff * valueLoss + _config.entropyCoeff * entropyLoss;

            // ---- Optimize ----
            _optimizer->zero_grad();
            totalLoss.backward();

            // Gradient norm clipping (prevents catastrophic updates)
            torch::nn::utils::clip_grad_norm_(
                _actorCritic->parameters(), _config.maxGradNorm);

            _optimizer->step();
        }
    }
}

// ============================================================================
// Main Training Loop
// ============================================================================

void PPOTrainer::train()
{
    std::cout << "\n"
              << "==============================================" << "\n"
              << "   PPO Training - P-OLA Smart Thermostat      " << "\n"
              << "==============================================" << "\n"
              << "  Total timesteps:  " << _config.totalTimesteps << "\n"
              << "  Rollout steps:    " << _config.rolloutSteps << "\n"
              << "  Mini-batch size:  " << _config.miniBatchSize << "\n"
              << "  PPO epochs:       " << _config.numEpochs << "\n"
              << "  Learning rate:    " << _config.learningRate << "\n"
              << "  Hidden dim:       " << _config.hiddenDim << "\n"
              << "  Reward weights:   comfort=" << _config.wComfort
              << "  economy=" << _config.wEconomy
              << "  gps=" << _config.wGps << "\n"
              << "  Device:           " << _device << "\n"
              << "==============================================" << "\n"
              << std::endl;

    while (_totalSteps < _config.totalTimesteps)
    {
        // 1. Collect rollout from environment
        collectRollout();

        // 2. PPO update
        update();

        _numRollouts++;

        // ---- Logging ----
        if (_numRollouts % _config.logInterval == 0)
        {
            double totalReward = 0.0;
            for (int i = 0; i < _config.rolloutSteps; ++i)
            {
                totalReward += _rollout.rewards[i];
            }
            const double avgReward = totalReward / _config.rolloutSteps;

            std::cout << "[PPO] Rollout " << _numRollouts
                      << " | Steps: " << _totalSteps
                      << "/" << _config.totalTimesteps
                      << " | Avg Reward: " << avgReward
                      << std::endl;

            if (avgReward > _bestAvgReward)
            {
                _bestAvgReward = avgReward;
            }
        }

        // ---- Periodic model checkpointing ----
        if (_numRollouts % _config.saveInterval == 0)
        {
            exportModel();
            std::cout << "[PPO] Checkpoint saved to: " << _config.modelSavePath << std::endl;
        }
    }

    // Final export
    exportModel();
    std::cout << "\n[PPO] Training complete!"
              << "\n[PPO] Best avg reward: " << _bestAvgReward
              << "\n[PPO] Model saved to:  " << _config.modelSavePath
              << std::endl;
}

void PPOTrainer::exportModel()
{
    _actorCritic->exportActor(_config.modelSavePath);
}
