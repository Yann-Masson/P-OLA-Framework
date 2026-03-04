/**
 * @file TrainingConfig.hpp
 * @brief Configuration and hyperparameters for PPO training of the thermostat
 * agent.
 *
 * All values in this struct are scientifically motivated hyperparameters.
 * In a paper, cite them as tunable parameters defining the agent "personality".
 */

#pragma once

#include <string>

namespace POLA::Training
{
    /**
     * @brief Normalization parameters for mapping raw sensor values to [0, 1].
     *
     * Each input dimension is normalized as: (value - offset) / scale.
     * Ranges are chosen to cover expected operational conditions with margin.
     */
    struct StateNorm
    {
        // ---- Current sensors ----
        static constexpr double tempIn_offset = 5.0,
                                tempIn_scale = 30.0; // [5, 35] °C
        static constexpr double price_offset = 0.0,
                                price_scale = 0.50; // [0, 0.50] $/kWh

        // ---- User position ----
        static constexpr double dist_offset = 0.0,
                                dist_scale = 50.0; // [0, 50] km
        static constexpr double vel_offset = 0.0,
                                vel_scale = 2.0; // [0, 2] km/min

        // ---- Weather forecast (per hour, repeated for each forecast step) ----
        static constexpr double forecastTemp_offset = -20.0,
                                forecastTemp_scale = 60.0; // [-20, 40] °C
        static constexpr double sunlight_offset = 0.0,
                                sunlight_scale = 100000.0; // [0, 100 000] lux

        // ---- User preferences ----
        static constexpr double prefTemp_offset = 15.0,
                                prefTemp_scale = 15.0; // [15, 30] °C

        static double normalize(const double val, const double offset,
                                const double scale)
        {
            return (val - offset) / (scale + 1e-8);
        }
    };

    /**
     * @brief Full configuration for PPO training of the smart thermostat
     * controller.
     *
     * Reward function uses a weighted multi-objective sum:
     *   R = -(w_comfort * C) - (w_economy * E) - (w_gps * G)
     *
     * where:
     *   C = (T_in - T_target)^2           (quadratic comfort penalty)
     *   E = price * heater_power           (energy cost penalty)
     *   G = arrival penalty if cold at home (GPS deadline penalty)
     */
    struct TrainingConfig
    {
        // ---- PPO Hyperparameters ----
        double learningRate = 3e-4; ///< Adam optimizer learning rate
        double gamma = 0.99; ///< Discount factor for future rewards
        double lambda = 0.95; ///< GAE lambda for advantage estimation
        double clipEpsilon = 0.2; ///< PPO surrogate clipping range [1-e, 1+e]
        double entropyCoeff = 0.5; ///< Entropy bonus to encourage exploration
        double valueCoeff = 0.5; ///< Critic loss weight in total loss
        double maxGradNorm = 0.5; ///< Gradient norm clipping threshold
        int numEpochs = 4; ///< PPO optimization epochs per rollout
        int rolloutSteps = 2048; ///< Environment steps per rollout collection
        int miniBatchSize = 64; ///< Mini-batch size for SGD updates
        int totalTimesteps = 1000000; ///< Total environment steps for full training

        // ---- Reward Function Weights (Scientific Hyperparameters) ----
        double wComfort = 0.8; ///< Comfort penalty weight (quadratic temp error)
        double wEconomy = 0.2; ///< Economy penalty weight (price × power coupling)

        // ---- Environment Parameters ----
        int episodeLength = 360; ///< Max steps per episode (6 hours at 1 min/step)
        double dt = 60.0; ///< Simulation time step in seconds

        // ---- Network Architecture ----
        int stateDim =
            42;
        ///< Number of state features: tempIn(1) + price(1) + dist(1) + vel(1)
                 ///< + weatherForecast(6×2=12) + userPrefs(2) + userSchedule(24)
        int actionDim = 1; ///< Number of actions (heater power)
        int hiddenDim = 64; ///< Hidden layer size for actor and critic

        // ---- Output / Logging ----
        std::string modelSavePath = "models/ai_model.pt";
        int logInterval = 10; ///< Rollouts between console log messages
        int saveInterval = 10; ///< Rollouts between model checkpoint saves
    };
} // namespace POLA::Training
