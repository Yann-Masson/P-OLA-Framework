/**
 * @file ActorCriticNoGPS.hpp
 * @brief Actor-Critic neural network for GPS-free PPO training (40-dim input).
 *
 * Identical architecture to ActorCriticPOLA, but exportActor() embeds the
 * 40-feature no-GPS normalization (no userDistanceKm / userVelocityKmMin).
 *
 * Architecture:
 *   Actor:  Linear(40→H) → ReLU → Linear(H→H) → ReLU → Linear(H→1)
 *   Critic: Linear(40→H) → ReLU → Linear(H→H) → ReLU → Linear(H→1)
 */

#pragma once

#include <string>
#include <torch/torch.h>

namespace POLA::Training::OLA
{
    struct ActorCriticOLAImpl : torch::nn::Module
    {
        /**
         * @param stateDim  Number of state features (default: 40 for no-GPS)
         * @param actionDim Number of action outputs (default: 1)
         * @param hiddenDim Hidden layer size (default: 64)
         */
        ActorCriticOLAImpl(int64_t stateDim = 40, int64_t actionDim = 1,
                           int64_t hiddenDim = 64);

        std::tuple<torch::Tensor, torch::Tensor> forward(const torch::Tensor& state);

        std::tuple<torch::Tensor, torch::Tensor, torch::Tensor>
        act(torch::Tensor state);

        std::tuple<torch::Tensor, torch::Tensor, torch::Tensor>
        evaluate(torch::Tensor states, const torch::Tensor& actionLogits);

        /**
         * @brief Export the trained actor as a TorchScript model with
         *        built-in 40-dim (no-GPS) normalization.
         *
         * State layout (column order, matching normalizeState() in
         * OLATrainingAgent):
         *   0:  tempIn           [5, 35] °C
         *   1:  electricityPrice [0, 0.50] $/kWh
         *   2..13 (pairs): weather.forecast[0..5]
         *       even cols: outdoorTemp  [-20, 40] °C
         *       odd  cols: sunlight lux [0, 100 000]
         *   14: userPreferences.minTemperature [15, 30] °C
         *   15: userPreferences.maxTemperature [15, 30] °C
         *   16..39: userSchedule.userPresent[0..23]  (0.0 or 1.0)
         */
        void exportActor(const std::string& path);

        void saveCheckpoint(const std::string& path);
        void loadCheckpoint(const std::string& path);

        // Actor layers
        torch::nn::Linear actor_fc1{nullptr}, actor_fc2{nullptr}, actor_out{nullptr};
        // Critic layers
        torch::nn::Linear critic_fc1{nullptr}, critic_fc2{nullptr},
                          critic_out{nullptr};
        // Learnable exploration log-std
        torch::Tensor log_std;

    private:
        int64_t _stateDim;
        int64_t _actionDim;
    };

    TORCH_MODULE(ActorCriticOLA);
} // namespace POLA::Training
