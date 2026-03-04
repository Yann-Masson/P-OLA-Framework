/**
 * @file RewardFunction.cpp
 * @brief Implementation of the multi-objective reward function.
 */

#include "RewardFunction.hpp"
#include "Interfaces/IUserComfortService.hpp"

#include <cmath>
#include <algorithm>
#include <iostream>

using namespace POLA::Training;
using namespace POLA::Interfaces;

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
    // ---- 2. Economy Penalty (Price × Action Coupling) ----
    // The agent is penalized proportionally to the electricity price and
    // its power output. This creates an incentive to shift heating load
    // to low-tariff periods ("thermal load shifting").
    const double economyPenalty = state.electricityPrice * heaterPower;

    // ---- 3. User comfort Penalty (Temporal Deadline) ----
    // A harsh binary penalty if the user arrives home to a cold house.
    // This forces the agent to develop a predictive strategy based on
    // the user's velocity vector, planning pre-heating in advance.
    auto comfortService = _provider.get<IUserComfortService>();
    const double userComfort = comfortService->recordComfort(state.tempIn);
    double comfortPenalty = std::abs(userComfort - 100.0); // 0 comfort → 100 penalty, 100 comfort → 0 penalty

    // ---- Weighted Multi-Objective Sum ----
    return -(_wComfort * comfortPenalty + _wEconomy * economyPenalty);
}
