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

RewardFunction::RewardFunction(const forge::ProviderRef &provider,
                               const TrainingConfig &config)
    : _provider(provider), _wComfort(config.wComfort),
      _wEconomy(config.wEconomy)
{
    std::cout << "[RewardFunction] Initialized with weights:\n"
        << "  Comfort: " << _wComfort << "\n"
        << "  Economy: " << _wEconomy << std::endl;
}

double RewardFunction::compute(const Common::AIState &state,
                               const double heaterPower,
                               const Common::AIState &nextState) const
{
    // ---- 2. Economy Penalty (Price × Action Coupling) ----
    // The agent is penalized proportionally to the electricity price and
    // its power output. This creates an incentive to shift heating load
    // to low-tariff periods ("thermal load shifting").
    const double economyPenalty = state.electricityPrice * heaterPower;

    // ---- 3. User comfort Penalty (Temporal Deadline / Thermal Excursion) ----
    // We convert the exponential 0-100 comfort score back into its underlying
    // difference (diff), and apply a quadratic penalty as documented.
    // userComfort = 100 * exp(-0.4 * diff) => diff = -ln(userComfort / 100) / 0.4
    const auto comfortService = _provider.get<IUserComfortService>();
    const auto userComfort = comfortService->recordComfort(state.tempIn);
    const auto comfortPenalty = std::abs(userComfort - 100.0); // 0 comfort → 100 penalty, 100 comfort → 0 penalty

    // Harsh penalty for dangerous overheating
    double overheatPenalty = 0.0;
    if (nextState.tempIn > 33.0)
    {
        double excess = nextState.tempIn - 33.0;
        overheatPenalty = 100.0 * excess * excess;
    }

    auto reward = -(_wComfort * comfortPenalty + _wEconomy * economyPenalty + overheatPenalty);

    return reward;
}
