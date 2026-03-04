/**
 * @file IAIModelNoGPS.hpp
 * @brief Interface for AI models that operate on GPS-free state inputs.
 *
 * Parallel to IAIModel, but accepts AIStateNoGPS (4 features) instead of
 * AIState (6 features). Used by SmartThermostatNoGPS and
 * PPOTrainingAgentNoGPS.
 */

#pragma once

#include "Common/AIStateNoGPS.hpp"

namespace POLA::Interfaces {

class IAIModelNoGPS {
public:
  virtual ~IAIModelNoGPS() = default;

  /**
   * @brief Predict heater power given the current no-GPS state.
   * @param state Current state (4 features, no GPS).
   * @return Heater power in [0, 1].
   */
  virtual double predict(const Common::AIStateNoGPS &state) = 0;
};

} // namespace POLA::Interfaces
