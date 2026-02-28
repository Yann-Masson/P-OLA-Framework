/**
 * @file Heater.cpp
 * @brief Implementation of the heater temperature factor.
 *
 * Uses a lumped-capacitance thermal model:
 *   dT = (power × P_max) × dt / C_thermal
 *
 *   P_max     = 2000 W (typical residential heater)
 *   C_thermal = 500,000 J/K (room + furniture thermal mass)
 */

#include "Heater.hpp"

#include <algorithm>

#include "Interfaces/IClock.hpp"
#include "Interfaces/IConsumptionService.hpp"

using namespace POLA::Simulation::TemperatureFactor;

double Heater::simulate(double insideTemperature)
{
    const double dtMs = _provider.get<Interfaces::IClock>()->getElapsedTime();
    const double dtSec = dtMs / 1000.0;
    if (dtSec <= 0.0) return 0.0;

    constexpr double maxPowerW = 2000.0;          // Watts
    constexpr double thermalCapacitance = 500000.0; // J/K

    const double heatOutputW = _power * maxPowerW;
    const double energyKWh = heatOutputW * dtSec / 3600000.0;  // W·s → kWh

    _provider.get<Interfaces::IConsumptionService>()->recordEnergy(energyKWh);

    return heatOutputW * dtSec / thermalCapacitance;
}

void Heater::setPower(const double power)
{
    _power = std::clamp(power, 0.0, 1.0);
}

double Heater::getPower() const
{
    return _power;
}
