#include "Heater.hpp"

#include "../consumptionService/IConsumptionService.hpp"

double Heater::simulate(double insideTemperature)
{
    // TODO: calculate the real value from the wanted temperature
    _provider.get<IConsumptionService>()->recordEnergy(0.1); //TODO: find the real value to record
    return 5.0;
}

void Heater::setWantedTemperature(const double wantedTemperature)
{
    _wantedTemperature = wantedTemperature;
}
