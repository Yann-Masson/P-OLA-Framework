/**
 * @file GPSService.cpp
 * @brief Implementation of the GPS data service.
 */

#include "GPSService.hpp"

using namespace POLA::Common;
using namespace POLA::Services::Inputs;

GPSData GPSService::getInput()
{
    return {
        .distanceKm = 10.0,
        .velocityKmMin = 60.0
    };
}
