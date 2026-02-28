/**
 * @file RewardFunction.hpp
 * @brief Multi-objective reward function for smart thermostat RL training.
 */

#pragma once

#include "Common/AIState.hpp"
#include "TrainingConfig.hpp"

namespace POLA::Training {

/**
 * @brief Computes the reward signal for the PPO agent.
 *
 * The reward is a weighted negative sum of three penalty components:
 *
 *   R = -(w_comfort * C) - (w_economy * E) - (w_gps * G)
 *
 * ## Comfort Penalty (C)
 * Quadratic deviation from target temperature:
 *   C = (T_in - T_target)^2
 *
 * A quadratic penalty models human metabolic discomfort, which increases
 * exponentially as the environment moves outside the ASHRAE comfort zone.
 * Small deviations (0.5°C) are mild; large deviations (4°C) are harsh.
 *
 * ## Economy Penalty (E)
 * Price-weighted power consumption:
 *   E = electricity_price * heater_power
 *
 * Directly couples energy cost with the agent's action, incentivizing
 * "thermal load shifting" — pre-heating during low-tariff periods.
 *
 * ## GPS Arrival Penalty (G)
 * Binary penalty triggered when the user arrives home to a cold house:
 *   G = 20 * max(0, T_target - T_in)   if distance < 0.1 km and deficit > 1°C
 *
 * Creates a temporal deadline for the agent, forcing predictive pre-heating
 * based on the user's GPS velocity vector.
 */
class RewardFunction {
public:
    explicit RewardFunction(const TrainingConfig& config);

    /**
     * @brief Compute reward for a single environment transition.
     * @param state     Current state before the action was taken
     * @param heaterPower Action taken by the agent (0.0 = off, 1.0 = max)
     * @param nextState  Resulting state after physics simulation
     * @return Scalar reward (always ≤ 0; closer to 0 is better)
     */
    [[nodiscard]] double compute(
        const Common::AIState& state,
        double heaterPower,
        const Common::AIState& nextState
    ) const;

private:
    double _wComfort;
    double _wEconomy;
    double _wGps;
};

} // namespace POLA::Training
