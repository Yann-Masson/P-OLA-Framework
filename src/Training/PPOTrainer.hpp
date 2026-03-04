/**
 * @file PPOTrainer.hpp
 * @brief Proximal Policy Optimization (PPO) trainer for the thermostat agent.
 */

#pragma once

#include <memory>
#include <vector>

#include <torch/torch.h>
#include <forge/provider.hpp>

#include "ActorCritic.hpp"
#include "RewardFunction.hpp"
#include "TrainingConfig.hpp"
#include "TrainingEnvironment.hpp"

namespace POLA::Training
{
    /**
     * @brief PPO trainer implementing the clipped surrogate objective.
     *
     * Training loop (per rollout):
     *  1. Collect N steps from the environment using the current policy
     *  2. Compute advantages using Generalized Advantage Estimation (GAE)
     *  3. Perform K epochs of clipped mini-batch PPO updates
     *  4. Repeat until totalTimesteps is reached
     *  5. Export the trained actor as a TorchScript model
     *
     * The clipped PPO objective prevents destructively large policy updates:
     *   L_clip = min(ratio * A, clip(ratio, 1-ε, 1+ε) * A)
     *
     * where ratio = π_new(a|s) / π_old(a|s) and A is the GAE advantage.
     */
    class PPOTrainer
    {
    public:
        /**
         * @brief Construct a PPO trainer with a fully configured provider.
         * @param provider Service provider with all simulation services
         * @param config Training configuration
         * @param seed Random seed for reproducibility
         */
        explicit PPOTrainer(
            forge::ProviderRef provider,
            const TrainingConfig& config,
            uint32_t seed = 42
        );

        /// Run the full training loop until totalTimesteps is reached.
        void train();

        /// Export the trained actor network as a TorchScript model.
        void exportModel();

    private:
        /// Normalize a raw AIState into a [0,1]-range tensor for the network.
        [[nodiscard]] torch::Tensor normalizeState(const Common::AIState& state) const;

        /// Collect one rollout of config.rolloutSteps transitions.
        void collectRollout();

        /// Perform PPO clipped update on the collected rollout data.
        void update();

        TrainingConfig _config;
        ActorCritic _actorCritic{nullptr};
        TrainingEnvironment _env;
        RewardFunction _rewardFn;
        std::unique_ptr<torch::optim::Adam> _optimizer;
        torch::Device _device = torch::kCPU;

        /// Rollout buffer: populated during collection, consumed during update.
        struct RolloutData
        {
            std::vector<torch::Tensor> states; ///< Normalized state tensors [stateDim]
            std::vector<float> actionLogits; ///< Sampled logits (pre-sigmoid)
            std::vector<float> logProbs; ///< log π_old(a|s)
            std::vector<float> values; ///< V(s) estimates (N+1 for bootstrap)
            std::vector<float> rewards; ///< r_t per step
            std::vector<bool> dones; ///< Episode termination flags

            void clear();
        } _rollout;

        // Training statistics
        int _totalSteps = 0;
        int _numRollouts = 0;
        double _bestAvgReward = -1e9;
    };
} // namespace POLA::Training
