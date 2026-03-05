/**
 * @file OLATrainingAgent.hpp
 * @brief PPO training agent for the GPS-free thermostat model (40-dim state).
 *
 * Mirrors POLATrainingAgent but:
 *  - Implements IAIModelNoGPS (accepts AIStateNoGPS)
 *  - Uses ActorCriticNoGPS (default stateDim = 40)
 *  - normalizeState() builds a 40-element tensor (no GPS features)
 */

#pragma once

#include "../TrainingConfig.hpp"
#include "ActorCriticOLA.hpp"


#include <forge/provider.hpp>
#include <memory>
#include <optional>

#include "Interfaces/IAIModel.hpp"
#include "Training/RewardFunction.hpp"

namespace POLA::Training::OLA
{
    class OLATrainingAgent : public Interfaces::IAIModel
    {
    public:
        OLATrainingAgent(const forge::ProviderRef& provider,
                         const TrainingConfig& config, uint32_t seed = 42);

        double predict(const Common::AIState& state) override;

    private:
        struct RolloutTransition
        {
            torch::Tensor stateTensor;
            float actionLogit;
            float logProb;
            float value;
            float roomTemperature;
            float reward;
            bool done;
        };

        ActorCriticOLA _actorCritic = nullptr;
        std::unique_ptr<torch::optim::Adam> _optimizer;
        torch::Device _device = torch::kCPU;

        TrainingConfig _config;
        forge::ProviderRef _provider;
        RewardFunction _rewardFn;

        std::vector<RolloutTransition> _rollout;

        std::optional<Common::AIState> _prevState;
        std::optional<RolloutTransition> _prevTransition;

        int _totalSteps = 0;
        int _numRollouts = 0;
        double _bestAvgReward = -1e9;

        torch::Tensor normalizeState(const Common::AIState& state) const;
        void updatePPO(const torch::Tensor& finalStateTensor);
        void exportModel();
    };
} // namespace POLA::Training::OLA
