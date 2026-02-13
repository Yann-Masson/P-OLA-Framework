#include "Heater.hpp"

double Heater::simulate()
{
    // TODO: calculate the real value from the wanted temperature
    // ? how to get the wanted temperature ?
    // TODO: call the ConsumptionService to add the consumption of the heater (from the provider)
    return 5.0;
}

void Heater::setWantedTemperature(double wantedTemperature)
{
	_wantedTemperature = wantedTemperature;
}
