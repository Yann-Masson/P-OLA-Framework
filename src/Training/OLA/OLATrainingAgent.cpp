/**
 * @file OLATrainingAgent.cpp
 * @brief Implementation of the GPS-free PPO training agent.
 *
 * State vector layout (24 dimensions, matching ActorCriticOLA.exportActor):
 *   [0..23] userSchedule.userPresent[0..23]
 */

#include <algorithm>
#include <filesystem>
#include <iostream>

#include "OLATrainingAgent.hpp"

#include "Common/DataTypes.hpp"
#include "Interfaces/IAIRecorder.hpp"
#include "Interfaces/IClock.hpp"
#include "Interfaces/IInputService.hpp"
#include "Simulation/Room/Room.hpp"

using namespace POLA::Training::OLA;
using namespace POLA::Common;
using namespace POLA::Interfaces;

// OLA state is 24 dims: schedule(24)
static constexpr int kOLAStateDim = 24;

OLATrainingAgent::OLATrainingAgent(const forge::ProviderRef &provider,
                                   const TrainingConfig &config,
                                   uint32_t seed)
    : _provider(provider), _config(config), _rewardFn(provider, config),
      _rng(seed)
{
    if (torch::cuda::is_available())
    {
        _device = torch::kCUDA;
        std::cout << "[OLA] Using CUDA device" << std::endl;
    }
    else if (torch::mps::is_available())
    {
        _device = torch::kMPS;
        std::cout << "[OLA] Using MPS device" << std::endl;
    }
    else
    {
        _device = torch::kCPU;
        std::cout << "[OLA] Using CPU device" << std::endl;
    }

    _actorCritic =
        ActorCriticOLA(kOLAStateDim, _config.actionDim, _config.hiddenDim);
    _actorCritic->to(_device);

    _optimizer = std::make_unique<torch::optim::Adam>(
        _actorCritic->parameters(),
        torch::optim::AdamOptions(_config.learningRate));

    _rollout.reserve(_config.rolloutSteps);

    // Resume from checkpoint if one exists
    const std::string ckptPath = _config.modelSavePath + ".ckpt";
    if (std::filesystem::exists(ckptPath))
    {
        try
        {
            _actorCritic->loadCheckpoint(ckptPath);
            std::cout << "[OLA] Resumed from checkpoint: " << ckptPath << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << "[OLA] Warning: failed to load checkpoint '" << ckptPath
                      << "': " << e.what() << " — starting from scratch."
                      << std::endl;
        }
    }
    else
    {
        std::cout << "[OLA] No checkpoint at '" << ckptPath
                  << "' — starting from scratch." << std::endl;
    }
}

torch::Tensor OLATrainingAgent::normalizeState(const AIState &state) const
{
    // Flatten to 24 floats matching ActorCriticOLA column layout
    std::vector<float> values;
    values.reserve(kOLAStateDim);

    // [0..23] userSchedule (24 hours, binary)
    for (const bool present : state.userSchedule.userPresent)
        values.push_back(present ? 1.0f : 0.0f);

    auto tensor = torch::tensor(
        values, torch::TensorOptions().dtype(torch::kFloat32).device(_device));

    return tensor.unsqueeze(0); // [1, 24]
}

AIState OLATrainingAgent::buildCurrentStateFromServices() const
{
    const auto room = _provider.get<Simulation::Room>();
    const auto energyPrice =
        _provider.get<IInputService<EnergyPriceData>>()->getInput();
    const auto weather = _provider.get<IInputService<WeatherData>>()->getInput();
    const auto userPref =
        _provider.get<IInputService<UserPreferenceData>>()->getInput();
    const auto gps = _provider.get<IInputService<GPSData>>()->getInput();
    const auto userSchedule =
        _provider.get<IInputService<UserScheduleData>>()->getInput();

    const double priceNow = energyPrice.pricesPerKwh.empty()
                                ? 0.0
                                : energyPrice.pricesPerKwh[0];

    return AIState{
        room->getTemperature(),
        priceNow,
        gps.distanceKm,
        gps.velocityKmMin,
        weather,
        userPref,
        userSchedule};
}

void OLATrainingAgent::resetEpisodeEnvironment(const double avgEpisodeTemp)
{
    // const auto room = _provider.get<Simulation::Room>();
    // const auto userPref =
    //     _provider.get<IInputService<UserPreferenceData>>()->getInput();

    // const double lowResetTemp = userPref.minTemperature - 2.0;
    // const double highResetTemp = userPref.maxTemperature + 2.0;
    // std::uniform_real_distribution<double> resetTempDist(lowResetTemp,
    //                                                      highResetTemp);
    // const double resetTemp = resetTempDist(_rng);

    // int phaseSteps = -1;
    // if (_config.randomizeEpisodeTimeOffset)
    // {
    //     const auto clock = _provider.get<IClock>();
    //     std::uniform_int_distribution<int> phaseStepsDist(0, 24 * 60 - 1);
    //     phaseSteps = phaseStepsDist(_rng);

    //     clock->reset();
    //     for (int i = 0; i < phaseSteps; ++i)
    //     {
    //         clock->simulate();
    //     }
    // }

    // room->reset(resetTemp);

    // if (_config.randomizeEpisodeTimeOffset)
    // {
    //     std::cout << "[OLA] Episode reset | Avg Temp: " << avgEpisodeTemp
    //               << " °C | Reset Temp: " << resetTemp
    //               << " °C | Time offset: " << phaseSteps << " min" << std::endl;
    // }
    // else
    // {
    //     std::cout << "[OLA] Episode reset | Reset Temp: " << resetTemp
    //               << " °C | Avg Temp: " << avgEpisodeTemp << " °C" << std::endl;
    // }
    // Option A: No room resets
    // Temperature drifts naturally based on physics throughout training
    // Episodes mark PPO update boundaries only
    std::cout << "[OLA] Episode completed | Avg Room Temp: " << avgEpisodeTemp
              << " °C | Continuing with natural temperature progression" << std::endl;
}

double OLATrainingAgent::predict(const AIState &currentState)
{
    AIState stateForAction = currentState;
    bool episodeEnded = false;
    double avgEpisodeTemp = 0.0;

    // 1. Reward the previous transition
    if (_prevState.has_value() && _prevTransition.has_value())
    {
        const float logit = _prevTransition->actionLogit;
        const double powerW = std::clamp(1.0 / (1.0 + std::exp(-logit)), 0.0, 1.0);

        // Use actual environment state for reward computation (not zeros)
        // OLA doesn't use GPS in its policy, but reward still needs real temp/price
        const double reward =
            _rewardFn.compute(_prevState.value(), powerW, currentState);

        const double timestamp =
            _provider.get<IClock>()->getElapsedTimeSinceStart();
        const double actualPower = std::clamp(powerW, 0.0, 1.0);

        // Record the full current state (OLA doesn't use GPS in policy, but we track it)
        const auto recorder = _provider.get<IAIRecorder>();
        recorder->record(timestamp, currentState, powerW, actualPower, reward);

        _prevTransition->reward = static_cast<float>(reward);
        _episodeTempSum += _prevTransition->roomTemperature;
        _episodeTempCount++;
        _episodeStep++;

        if (_episodeStep >= _config.episodeLength)
        {
            _prevTransition->done = true;
            episodeEnded = true;
            avgEpisodeTemp = _episodeTempCount > 0
                                 ? _episodeTempSum / static_cast<double>(_episodeTempCount)
                                 : 0.0;
            _episodeStep = 0;
            _episodeTempSum = 0.0;
            _episodeTempCount = 0;
        }

        _rollout.push_back(_prevTransition.value());

        if (_rollout.size() == static_cast<size_t>(_config.rolloutSteps))
        {
            if (!episodeEnded)
            {
                _rollout.back().done = true;
            }

            const auto bootstrapStateTensor = normalizeState(stateForAction);
            const auto room = _provider.get<Simulation::Room>();
            updatePPO(bootstrapStateTensor);
            _rollout.clear();

            if (!episodeEnded)
            {
                _episodeTempSum = room->getTemperature();
                _episodeTempCount = 1;
            }
        }

        if (episodeEnded)
        {
            resetEpisodeEnvironment(avgEpisodeTemp);
            _prevState.reset();
            _prevTransition.reset();
            stateForAction = buildCurrentStateFromServices();
        }
    }

    // 2. Sample action from current policy
    const auto stateTensor = normalizeState(stateForAction);
    _actorCritic->eval();
    auto [actionLogit, logProb, value] = _actorCritic->act(stateTensor);

    const double rawLogit = actionLogit.item<float>();
    const double action = torch::sigmoid(actionLogit).item<double>();
    const double powerW = std::clamp(action, 0.0, 1.0);

    // 3. Store transition
    _prevState = stateForAction;

    RolloutTransition transition;
    const auto room = _provider.get<Simulation::Room>();
    transition.stateTensor = stateTensor.squeeze(0);
    transition.actionLogit = static_cast<float>(rawLogit);
    transition.logProb = logProb.item<float>();
    transition.value = value.item<float>();
    transition.roomTemperature = static_cast<float>(room->getTemperature());
    transition.reward = 0.0f;
    transition.done = false;

    _prevTransition = transition;
    _totalSteps++;

    return powerW;
}

void OLATrainingAgent::updatePPO(const torch::Tensor &finalStateTensor)
{
    _actorCritic->train();

    auto [_, __, lastValueTensor] = _actorCritic->act(finalStateTensor);
    const float lastValue = lastValueTensor.item<float>();

    const int N = static_cast<int>(_rollout.size());

    std::vector<float> advantages(N, 0.0f);
    std::vector<float> returns(N, 0.0f);

    float lastGAE = 0.0f;
    for (int t = N - 1; t >= 0; --t)
    {
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

    for (const auto &r : _rollout)
    {
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

    for (int epoch = 0; epoch < _config.numEpochs; ++epoch)
    {
        auto indices = torch::randperm(
            N, torch::TensorOptions().dtype(torch::kLong).device(_device));

        for (int start = 0; start < N; start += _config.miniBatchSize)
        {
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

    if (_numRollouts % _config.logInterval == 0)
    {
        double totalReward = 0.0;
        double totalTemp = 0.0;
        for (int i = 0; i < N; ++i)
        {
            totalReward += rewards[i];
            totalTemp += _rollout[i].roomTemperature;
        }
        const double avgReward = totalReward / N;
        const double avgTemp = totalTemp / N;

        std::cout << "[OLA] Rollout " << _numRollouts << " | Steps: " << _totalSteps
                  << "/" << _config.totalTimesteps << " | Avg Reward: " << avgReward
                  << " | Avg Temp: " << avgTemp << " \u00b0C" << std::endl;

        if (avgReward > _bestAvgReward)
            _bestAvgReward = avgReward;
    }

    if (_numRollouts % _config.saveInterval == 0)
    {
        exportModel();
        std::cout << "[OLA] Checkpoint saved to: " << _config.modelSavePath
                  << std::endl;
    }
}

void OLATrainingAgent::exportModel()
{
    _actorCritic->exportActor(_config.modelSavePath);
    _actorCritic->saveCheckpoint(_config.modelSavePath + ".ckpt");
}
