/**
 * @file IEnvironmentControl.hpp
 * @brief Interface for controlling the simulation environment (e.g., resets).
 *
 * This is primarily used by the Reinforcement Learning agent to reset the
 * room temperature and simulation clock when a training episode finishes.
 */

#pragma once

namespace POLA::Interfaces {

class IEnvironmentControl {
public:
  virtual ~IEnvironmentControl() = default;

  /**
   * @brief Reset the environment to start a new episode.
   *
   * This typically involves resetting the clock and randomizing
   * the starting room temperature.
   */
  virtual void resetEnvironment() = 0;
};

} // namespace POLA::Interfaces
