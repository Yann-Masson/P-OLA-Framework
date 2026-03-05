/**
 * @file ActorCriticPOLA.hpp
 * @brief Actor-Critic neural network for PPO training.
 */

#pragma once

#include <string>
#include <torch/torch.h>


namespace POLA::Training::POLA
{
    /**
     * @brief Actor-Critic network with separate actor and critic pathways.
     *
     * The Actor outputs a logit for heater power. During training, Gaussian noise
     * is added for exploration; during inference, the deterministic sigmoid(logit)
     * is used directly.
     *
     * The Critic outputs a state-value estimate V(s) used for GAE advantage
     * computation during PPO training.
     *
     * Architecture (separate networks, no shared weights):
     *   Actor:  Linear(6→H) → ReLU → Linear(H→H) → ReLU → Linear(H→1)
     *   Critic: Linear(6→H) → ReLU → Linear(H→H) → ReLU → Linear(H→1)
     *
     * Weights are initialized with orthogonal initialization (standard for PPO).
     */
    struct ActorCriticPOLAImpl : torch::nn::Module
    {
        /**
         * @param stateDim  Number of state features (default: 6)
         * @param actionDim Number of action outputs (default: 1)
         * @param hiddenDim Hidden layer size (default: 64)
         */
        ActorCriticPOLAImpl(int64_t stateDim, int64_t actionDim, int64_t hiddenDim = 64);

        /**
         * @brief Forward pass returning raw actor logit and value estimate.
         * @param state Normalized state tensor [batch, stateDim]
         * @return (action_logit [batch, actionDim], value [batch, 1])
         */
        std::tuple<torch::Tensor, torch::Tensor> forward(torch::Tensor state);

        /**
         * @brief Sample an action for rollout data collection.
         *
         * Samples from a Gaussian in logit space: a ~ N(mean_logit, exp(log_std)).
         * The caller applies sigmoid to get the actual environment action in [0, 1].
         *
         * @param state Normalized state tensor [1, stateDim]
         * @return (sampled_logit, log_probability, state_value)
         */
        std::tuple<torch::Tensor, torch::Tensor, torch::Tensor>
        act(torch::Tensor state);

        /**
         * @brief Re-evaluate stored actions under the current policy (for PPO
         * update).
         * @param states       [batch, stateDim] — normalized states from rollout
         * @param actionLogits [batch, actionDim] — stored pre-sigmoid sampled logits
         * @return (new_log_probs [batch], new_values [batch], entropy [batch])
         */
        std::tuple<torch::Tensor, torch::Tensor, torch::Tensor>
        evaluate(torch::Tensor states, torch::Tensor actionLogits);

        /**
         * @brief Export the trained actor as a self-contained TorchScript model.
         *
         * The exported model includes:
         * - Input normalization (raw AIState values → [0, 1])
         * - Actor network forward pass
         * - Sigmoid activation on output
         *
         * This allows AIModel to load it directly for inference without
         * needing to know about normalization parameters.
         *
         * @param path File path for the saved TorchScript model
         */
        void exportActor(const std::string& path);

        /**
         * @brief Save the full network weights to a binary checkpoint file.
         *
         * Unlike exportActor() which produces a self-contained TorchScript model
         * for inference, this saves raw weights so training can be resumed later.
         *
         * @param path File path for the checkpoint (e.g. "models/actor_critic.ckpt")
         */
        void saveCheckpoint(const std::string& path);

        /**
         * @brief Load network weights from a checkpoint saved by saveCheckpoint().
         * @param path File path of the checkpoint to load
         */
        void loadCheckpoint(const std::string& path);

        // Actor network layers
        torch::nn::Linear actor_fc1{nullptr}, actor_fc2{nullptr}, actor_out{nullptr};
        // Critic network layers
        torch::nn::Linear critic_fc1{nullptr}, critic_fc2{nullptr},
                          critic_out{nullptr};
        // Learnable exploration noise (log standard deviation in logit space)
        torch::Tensor log_std;

    private:
        int64_t _stateDim;
        int64_t _actionDim;
    };

    TORCH_MODULE(ActorCriticPOLA);
} // namespace POLA::Training
