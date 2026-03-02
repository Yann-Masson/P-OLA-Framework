/**
 * @file GPSService.cpp
 * @brief Implementation of the GPS data service.
 */

#include "GPSService.hpp"
#include "Interfaces/IClock.hpp"
#include "Simulation/DataManager/DataManager.hpp"

using namespace POLA::Common;
using namespace POLA::Services::Inputs;
using namespace POLA::Interfaces;
using namespace POLA::Simulation;

GPSData GPSService::getInput()
{
    const auto clock = _provider.get<IClock>();
    const auto currentTime = clock->getElapsedTimeSinceStart();
    const auto dataManager = _provider.get<DataManager>();
    DataPoint dp = dataManager->getDataPointForTime(currentTime);

    return {
        .distanceKm = dp.user_distance,
        .velocityKmMin = dp.user_velocity
    };
}
