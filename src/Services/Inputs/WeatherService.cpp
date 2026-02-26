/**
 * @file WeatherService.cpp
 * @brief Implementation of the weather data service.
 */

#include "WeatherService.hpp"

using namespace POLA::Common;
using namespace POLA::Services::Inputs;

WeatherData WeatherService::getInput()
{
    return {
        .outTemperature = 20.0,
        .sunlightIntensity = 1000.0
    };
}
