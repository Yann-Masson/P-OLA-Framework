#include "ConsumptionService.hpp"

ConsumptionService::ConsumptionService(dic::ServiceProviderRef provider):
	_provider(provider)
{
}

void ConsumptionService::recordEnergy(double kWh)
{
	double pricePerKWh = 0.15; // TODO: retrieve this from the PriceService in the provider
	// double pricePerKwh = _provider.get<PriceService>().getCurrentPricePerKWh();
	_totalEnergyKWh += kWh;
	_totalCost += kWh * pricePerKWh;
}

double ConsumptionService::getTotalEnergyKWh() const
{
	return _totalEnergyKWh;
}

double ConsumptionService::getTotalCost() const
{
	return _totalCost;
}

void ConsumptionService::reset()
{
	_totalEnergyKWh = 0.0;
	_totalCost = 0.0;
}
