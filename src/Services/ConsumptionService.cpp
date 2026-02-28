/**
 * @file ConsumptionService.cpp
 * @brief Implementation of the energy consumption tracking service.
 */

#include "ConsumptionService.hpp"

#include "Interfaces/IInputService.hpp"
#include "Common/DataTypes.hpp"

using namespace POLA::Services;

ConsumptionService::ConsumptionService(const forge::ProviderRef& provider) : _provider(provider)
{
}

void ConsumptionService::recordEnergy(const double kWh)
{
    const double pricePerKWh = _provider.get<Interfaces::IInputService<Common::EnergyPriceData>>()->getInput().pricesPerKwh[0]; // Get current price from the energy price service
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
