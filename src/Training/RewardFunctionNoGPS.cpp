/**
 * @file RewardFunctionNoGPS.cpp
 * @brief Implementation of the GPS-free reward function.
 */

#include "RewardFunctionNoGPS.hpp"
#include "Interfaces/IUserComfortService.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "Common/DataTypes.hpp"
#include "Interfaces/IClock.hpp"
#include "Interfaces/IInputService.hpp"

using namespace POLA::Training;
using namespace POLA::Interfaces;

RewardFunctionNoGPS::RewardFunctionNoGPS(const forge::ProviderRef &provider,
                                         const TrainingConfig &config)
    : _provider(provider), _wComfort(config.wComfort),
      _wEconomy(config.wEconomy) {
  std::cout << "[RewardFunctionNoGPS] Initialized with weights:\n"
            << "  Comfort: " << _wComfort << "\n"
            << "  Economy: " << _wEconomy << std::endl;
}

double
RewardFunctionNoGPS::compute(const Common::AIStateNoGPS &state,
                             const double heaterPower,
                             const Common::AIStateNoGPS &nextState) const {
  // ---- Economy Penalty (Price × Action Coupling) ----
  const double economyPenalty = state.electricityPrice * heaterPower;

  // ---- Comfort Penalty ----
  const auto comfortService = _provider.get<IUserComfortService>();
  const auto userComfort = comfortService->recordComfort(state.tempIn);
  const auto comfortPenalty = std::abs(userComfort - 100.0);

  // ---- Overheating Safety Penalty ----
  double overheatPenalty = 0.0;
  if (nextState.tempIn > 33.0) {
    const double excess = nextState.tempIn - 33.0;
    overheatPenalty = 100.0 * excess * excess;
  }

  return -(_wComfort * comfortPenalty + _wEconomy * economyPenalty +
           overheatPenalty);
}
