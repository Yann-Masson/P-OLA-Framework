/**
 * @file Heater.cpp
 * @brief Implementation of the heater temperature factor.
 */

#include "Heater.hpp"

#include "Interfaces/IConsumptionService.hpp"

using namespace POLA::Simulation::TemperatureFactor;

double Heater::simulate(double insideTemperature)
{
    // TODO: calculate the real value from the wanted temperature
    _provider.get<Interfaces::IConsumptionService>()->recordEnergy(0.1); //TODO: find the real value to record
    return 5.0;
}

void Heater::setWantedTemperature(const double wantedTemperature)
{
    _wantedTemperature = wantedTemperature;
}
