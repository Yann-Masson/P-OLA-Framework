/**
 * @file GPSService.cpp
 * @brief Implementation of the GPS data service.
 */

#include "GPSService.hpp"

namespace POLA::Services::Inputs {

Common::GPSData GPSService::getInput()
{
    return {
        .distanceKm = 10.0,
        .velocityKmMin = 60.0
    };
}

} // namespace POLA::Services::Inputs

