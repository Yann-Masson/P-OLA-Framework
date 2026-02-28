/**
 * @file TrainingEnvironment.hpp
 * @brief Self-contained simulation environment for RL training.
 */

#pragma once

#include <random>
#include <tuple>

#include "Common/AIState.hpp"
#include "RewardFunction.hpp"
#include "TrainingConfig.hpp"

namespace POLA::Training {

/**
 * @brief Lightweight training environment simulating room thermodynamics.
 *
 * This environment is independent of the forge DI container and runs
 * millions of steps efficiently during PPO training.
 *
 * Each episode simulates a 6-hour window where:
 * - A user is away from home and returns at a random time
 * - Outdoor temperature slowly drifts with daily variation
 * - Electricity prices follow a sinusoidal time-of-day pattern
 * - The agent controls heater power [0, 1] each minute
 *
 * Room thermal dynamics use a lumped-capacitance model:
 *   T_next = T_current + dt * (Q_heater - Q_loss) / C_thermal
 *
 * where:
 *   Q_heater = power × P_max         (heater heat output in W)
 *   Q_loss   = U_total × (T_in - T_out) (conductive heat loss in W)
 *   C_thermal = effective thermal capacitance (J/K)
 */
class TrainingEnvironment {
public:
    explicit TrainingEnvironment(const TrainingConfig& config, uint32_t seed = 42);

    /// Reset the environment for a new episode with randomized scenario.
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
    TrainingConfig _config;
    RewardFunction _rewardFn;
    std::mt19937 _rng;

    // Total heat loss coefficient (sum of all walls + windows)
    double _totalConductance = 0.0;

    // ---- Episode state (evolves each step) ----
    int    _step           = 0;
    double _tempIn         = 20.0;
    double _tempOut        = 5.0;
    double _targetTemp     = 22.0;
    double _price          = 0.15;
    double _gpsDistance     = 10.0;
    double _gpsVelocity    = 0.0;

    // ---- Scenario parameters (randomized per episode) ----
    double _initialDistance = 10.0;
    int    _returnStep     = 60;       ///< Step when user starts driving home
    double _returnSpeed    = 1.0;      ///< User travel speed (km/min)
    double _priceBase      = 0.15;     ///< Base electricity price
    double _priceAmplitude = 0.10;     ///< Amplitude of price oscillation
    double _pricePhase     = 0.0;      ///< Random phase offset for pricing
    double _tempOutBase    = 5.0;      ///< Base outdoor temperature

    /// Update GPS, weather, and price dynamics for the current step.
    void updateDynamics();

    /// Compute electricity price at the current simulation time.
    [[nodiscard]] double computePrice() const;
};

} // namespace POLA::Training
