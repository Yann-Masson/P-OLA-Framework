/**
 * @file PPOTrainingAgent.hpp
 * @brief Inverse RL implementation where the PPO Trainer acts as the AI Model.
 */

#pragma once

#include "ActorCritic.hpp"
#include "Interfaces/IAIModel.hpp"
#include "Interfaces/IEnvironmentControl.hpp"
#include "RewardFunction.hpp"
#include "TrainingConfig.hpp"

#include <forge/provider.hpp>
#include <memory>
#include <optional>
#include <random>

namespace POLA::Training
{
    /**
     * @brief An Inverse RL agent that looks like an IAIModel to the simulation.
     *
     * Instead of an external loop calling `env.step()`, the simulation calls
     * `predict()` on this agent. Inside `predict()`, the agent:
     * 1. Rewards the previous action.
     * 2. Stores the transition.
     * 3. Trains PPO if the rollout buffer is full.
     * 4. Returns the next action for the simulation to apply.
     */
    class PPOTrainingAgent : public Interfaces::IAIModel
    {
    public:
        /**
         * @brief Construct a new PPOTrainingAgent
         *
         * @param provider The DI provider (used to get IEnvironmentControl)
         * @param config Training configuration parameters
         * @param seed Random seed for exploration and env resets
         */
        PPOTrainingAgent(const forge::ProviderRef& provider,
                         const TrainingConfig& config, uint32_t seed = 42);

        /**
         * @brief Predict the next action, but also perform learning!
         *
         * @param state The current simulation state
         * @return double The chosen heater power [0, 1]
         */
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

        // Network and optimizer
        ActorCritic _actorCritic = nullptr;
        std::unique_ptr<torch::optim::Adam> _optimizer;
        torch::Device _device = torch::kCPU;

        // Configuration and services
        TrainingConfig _config;
        forge::ProviderRef _provider;
        // std::shared_ptr<Interfaces::IEnvironmentControl> _envControl;
        RewardFunction _rewardFn;
        // std::mt19937 _rng;
        // std::uniform_real_distribution<double> _tempInDist;

        // Rollout buffer
        std::vector<RolloutTransition> _rollout;

        // State tracking across predict() calls
        std::optional<Common::AIState> _prevState;
        std::optional<RolloutTransition> _prevTransition;

        int _totalSteps = 0;
        int _numRollouts = 0;
        double _bestAvgReward = -1e9;

        // Internal methods
        torch::Tensor normalizeState(const Common::AIState& state) const;
        void updatePPO(const torch::Tensor& finalStateTensor);
        void exportModel();
    };
} // namespace POLA::Training
