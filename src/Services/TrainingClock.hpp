/**
 * @file TrainingClock.hpp
 * @brief Deterministic clock for RL training with fixed time steps.
 *
 * Unlike the real-time Clock (which measures wall-clock time), this clock
 * advances by a fixed dt every time simulate() is called. This ensures
 * consistent physics regardless of training speed.
 */

#pragma once

#include "Interfaces/IClock.hpp"

namespace POLA::Services {

class TrainingClock : public Interfaces::IClock {
public:
  /**
   * @brief Construct a training clock with a fixed time step.
   * @param fixedDtSeconds Simulated seconds per step (default: 60 = 1 minute)
   */
  explicit TrainingClock(double fixedDtSeconds = 60.0);

  /// Advance simulation time by one fixed dt.
  void simulate() override;

  /// Reset time back to zero.
  void reset() override;

  /// Get total simulated time since start (seconds).
  [[nodiscard]] uint32_t getElapsedTimeSinceStart() const override;

  /// Get simulated time for the last step (fixed dt).
  [[nodiscard]] uint32_t getElapsedTime() const override;

private:
  uint32_t _fixedDt;       ///< Fixed time step in seconds
  uint32_t _totalTime = 0; ///< Total simulated time since start
};

} // namespace POLA::Services
