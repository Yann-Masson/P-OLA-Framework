/**
 * @file TrainingEnvironment.hpp
 * @brief Full simulation environment for RL training using the Provider system.
 */

#pragma once

#include <random>
#include <tuple>
#include <memory>

#include <forge/provider.hpp>

#include "Common/AIState.hpp"
#include "RewardFunction.hpp"
#include "TrainingConfig.hpp"

namespace POLA::Training {

/**
 * @brief Training environment using the full P-OLA simulation stack.
 *
 * This environment uses the real Room, services, and temperature factors
 * from the simulation, providing a high-fidelity training environment
 * that matches the actual deployment conditions.
 *
 * Each episode:
 * - Uses the real Room thermal dynamics with Walls, Windows, and Heater
 * - Gets state from actual input services (GPS, Weather, EnergyPrice, etc.)
 * - Controls heater power through the Heater temperature factor
 * - Advances simulation time using the Clock service
 * - The agent learns to control heater power [0, 1] optimally
 */
class TrainingEnvironment {
public:
    /**
     * @brief Construct a training environment with the full simulation provider.
     * @param provider Service provider with all simulation services configured
     * @param config Training configuration (reward weights, episode length, etc.)
     * @param seed Random seed for episode initialization
     */
    explicit TrainingEnvironment(
        forge::Provider provider,
        const TrainingConfig& config,
        uint32_t seed = 42
    );

    /// Reset the environment for a new episode with randomized initial state.
    Common::AIState reset();

    /**
     * @brief Execute one time step with the given heater power.
     * @param heaterPower Heater power action in [0, 1]
     * @return (nextState, reward, done)
     */
    std::tuple<Common::AIState, double, bool> step(double heaterPower);

    /// Get the current state without advancing the simulation.
    [[nodiscard]] Common::AIState getState() const;

private:
    forge::Provider _provider;
    TrainingConfig _config;
    RewardFunction _rewardFn;
    std::mt19937 _rng;

    // Episode tracking
    int _step = 0;
};

} // namespace POLA::Training
