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
    DataPoint dp = dataManager->getDataPointForTime(currentTime);
    std::cout << "[EnergyPriceService] Providing energy price data for time " << dp.timestamp << "s: $" << dp.price_per_kWh << " per kWh" << std::endl;
    return EnergyPriceData{dp.price_per_kWh};
}
