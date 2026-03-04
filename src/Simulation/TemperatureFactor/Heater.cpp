/**
 * @file Heater.cpp
 * @brief Implementation of the heater temperature factor.
 *
 * Uses a lumped-capacitance thermal model:
 *   Q_heater = power × P_max  (Watts of heat delivered to the room)
 *
 *   P_max     = configurable (default 2000 W, typical residential heater)
 *
 * The simulate() method returns a NEGATIVE value because the Room's formula
 * sums all factors as heat LOSS. A negative loss = heat gain.
 *
 * Energy consumption is recorded in kWh via the IConsumptionService.
 */

#include "Heater.hpp"

#include <algorithm>

#include "Interfaces/IClock.hpp"
#include "Interfaces/IConsumptionService.hpp"

using namespace POLA::Simulation::TemperatureFactor;
using namespace POLA::Interfaces;

Heater::Heater(const forge::ProviderRef &provider, const double maxPowerW)
    : ATemperatureFactor(provider), _maxPowerW(maxPowerW) {}

double Heater::simulate(const double insideTemperature) {
  double currentPower = _power;
  if (insideTemperature >= 35.0) {
    currentPower = 0.0;
  }

  const double heatOutputW = currentPower * _maxPowerW;

  // Record energy consumption: convert Watts over elapsed seconds to kWh
  const auto clock = _provider.get<IClock>();
  const double dtSeconds = clock->getElapsedTime();
  const double energyKWh = (heatOutputW * dtSeconds) / 3'600'000.0;
  _provider.get<IConsumptionService>()->recordEnergy(energyKWh);

  // Return negative because Room sums factors as heat LOSS.
  // A heater is heat GAIN, so negative loss.
  return -heatOutputW;
}

void Heater::setPower(const double power) {
  _power = std::clamp(power, 0.0, 1.0);
}

double Heater::getPower() const { return _power; }

void Heater::reset() { _power = 0.0; }
