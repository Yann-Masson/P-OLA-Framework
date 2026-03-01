/**
 * @file EnergyPriceService.cpp
 * @brief Implementation of the energy price data service.
 */

#include "EnergyPriceService.hpp"

using namespace POLA::Common;
using namespace POLA::Services::Inputs;
using namespace POLA::Interfaces;
using namespace POLA::Simulation;

EnergyPriceData EnergyPriceService::getInput()
{
    auto dataManager = _provider.get<Simulation::DataManager>();
    auto clock = _provider.get<IClock>();
    uint32_t currentTime = clock->getElapsedTimeSinceStart();
    EnergyPriceData energyPriceData;
    for (int i = 0; i < PRICES_LENGTH; i++) {
        DataPoint dp = dataManager->getDataPointForTime(currentTime + i * 3600);
        energyPriceData.pricesPerKwh.push_back(dp.price_per_kWh);
    }
    return energyPriceData;
}
