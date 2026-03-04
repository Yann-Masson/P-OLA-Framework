/**
 * @file RewardFunctionNoGPS.hpp
 * @brief Reward function for the GPS-free thermostat agent.
 *
 * Identical in logic to RewardFunction but typed against AIStateNoGPS.
 * Since the existing reward function already has no GPS penalty, this is
 * a clean drop-in — only the state type changes.
 */

#pragma once

#include "Common/AIStateNoGPS.hpp"
#include "TrainingConfig.hpp"
#include "forge/provider.hpp"

namespace POLA::Training {
/**
 * @brief Computes the reward signal for the GPS-free PPO agent.
 *
 * R = -(w_comfort * C) - (w_economy * E)
 *
 * ## Comfort Penalty (C)
 * Maps directly from UserComfortService (0–100 score).
 *
 * ## Economy Penalty (E)
 * E = electricity_price * heater_power
 */
class RewardFunctionNoGPS {
public:
  explicit RewardFunctionNoGPS(const forge::ProviderRef &provider,
                               const TrainingConfig &config);

  /**
   * @brief Compute reward for a single environment transition.
   * @param state        Current state before the action
   * @param heaterPower  Action taken by the agent [0, 1]
   * @param nextState    Resulting state after physics simulation
   * @return Scalar reward (always ≤ 0; closer to 0 is better)
   */
  [[nodiscard]] double compute(const Common::AIStateNoGPS &state,
                               double heaterPower,
                               const Common::AIStateNoGPS &nextState) const;

private:
  double _wComfort;
  double _wEconomy;
  forge::ProviderRef _provider;
};
} // namespace POLA::Training
