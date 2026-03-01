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
    // TODO: calculate the real value from the wanted temperature
    _provider.get<Interfaces::IConsumptionService>()->recordEnergy(0.1); //TODO: find the real value to record
    return 5.0;
}

void Heater::setPower(const double power)
{
    _power = std::clamp(power, 0.0, 1.0);
}

double Heater::getPower() const
{
    return _power;
}
