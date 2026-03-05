/**
 * @file RewardFunction.cpp
 * @brief Implementation of the multi-objective reward function.
 */

#include "RewardFunction.hpp"
#include "Interfaces/IUserComfortService.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "Common/DataTypes.hpp"
#include "Interfaces/IClock.hpp"
#include "Interfaces/IInputService.hpp"

using namespace POLA::Training;
using namespace POLA::Interfaces;

RewardFunction::RewardFunction(const forge::ProviderRef& provider,
                               const TrainingConfig& config)
    : _provider(provider), _wComfort(config.wComfort),
      _wEconomy(config.wEconomy)
{
    std::cout << "[RewardFunction] Initialized with weights:\n"
              << "  Comfort: " << _wComfort << "\n"
              << "  Economy: " << _wEconomy << std::endl;
}

double RewardFunction::compute(const Common::AIState& state,
                               const double heaterPower,
                               const Common::AIState& nextState) const
{
    // ---- 2. Economy Penalty (Price × Action Coupling) ----
    // The agent is penalized proportionally to the electricity price and
    // its power output. This creates an incentive to shift heating load
    // to low-tariff periods ("thermal load shifting").
    const double economyPenalty = state.electricityPrice * heaterPower;
    // Normalize to [0, 1] using expected ranges:
    // electricityPrice in [1500, 3000], heaterPower in [0, 1]
    // => economyPenalty in [0, 3000]
    const double economyPenaltyNorm =
        std::clamp(economyPenalty / 3000.0, 0.0, 1.0);

    // ---- 3. User comfort Penalty (Temporal Deadline / Thermal Excursion) ----
    // We convert the exponential 0-100 comfort score back into its underlying
    const auto comfortService = _provider.get<IUserComfortService>();
    const auto userComfort = comfortService->recordComfort(state.tempIn);
    const auto comfortPenalty = std::abs(userComfort - 100.0); // 0 comfort → 100 penalty, 100 comfort → 0 penalty
    // Normalize to [0, 1]
    const double comfortPenaltyNorm = std::clamp(comfortPenalty / 100.0, 0.0, 1.0);

    // Harsh penalty for dangerous overheating
    double overheatPenalty = 0.0;
    if (nextState.tempIn > 33.0)
    {
        double excess = nextState.tempIn - 33.0;
        overheatPenalty = 10.0 * excess * excess;
    }

    auto reward = -(_wComfort * comfortPenaltyNorm + _wEconomy * economyPenaltyNorm + overheatPenalty);

    return reward;
}
