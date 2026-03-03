/**
 * @file RewardFunction.cpp
 * @brief Implementation of the multi-objective reward function.
 */

#include "RewardFunction.hpp"

#include <cmath>
#include <algorithm>
#include <iostream>

using namespace POLA::Training;

RewardFunction::RewardFunction(const forge::ProviderRef& provider,
                               const TrainingConfig& config)
    : _provider(provider)
      , _wComfort(config.wComfort)
      , _wEconomy(config.wEconomy)
      , _wGps(config.wGps)
{
    std::cout << "[RewardFunction] Initialized with weights:\n"
        << "  Comfort: " << _wComfort << "\n"
        << "  Economy: " << _wEconomy << "\n"
        << "  GPS:     " << _wGps << std::endl;
}

double RewardFunction::compute(
    const Common::AIState& state,
    const double heaterPower,
    const Common::AIState& nextState) const
{
    // ---- 1. Comfort Penalty (Squared Error) ----
    // Quadratic penalty simulates how human discomfort scales non-linearly
    // with temperature deviation from the target (ASHRAE comfort zone).
    const double tempError = nextState.tempIn - state.targetTemp;
    const double comfortPenalty = tempError * tempError;

    // ---- 2. Economy Penalty (Price × Action Coupling) ----
    // The agent is penalized proportionally to the electricity price and
    // its power output. This creates an incentive to shift heating load
    // to low-tariff periods ("thermal load shifting").
    const double economyPenalty = state.electricityPrice * heaterPower;

    // ---- 3. GPS Arrival Penalty (Temporal Deadline) ----
    // A harsh binary penalty if the user arrives home to a cold house.
    // This forces the agent to develop a predictive strategy based on
    // the user's velocity vector, planning pre-heating in advance.
    double gpsPenalty = 0.0;
    if (nextState.gpsDistance < 0.1)
    {
        // User is home or within 100 meters
        const double deficit = state.targetTemp - nextState.tempIn;
        if (deficit > 1.0)
        {
            // Aggressive penalty scaled by temperature deficit
            gpsPenalty = deficit * 20.0;
        }
    }

    // ---- Weighted Multi-Objective Sum ----
    return -(_wComfort * comfortPenalty + _wEconomy * economyPenalty + _wGps * gpsPenalty);
}
