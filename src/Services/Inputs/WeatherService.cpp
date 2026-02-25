/**
 * @file WeatherService.cpp
 * @brief Implementation of the weather data service.
 */

#include "WeatherService.hpp"

namespace POLA::Services::Inputs {

Common::WeatherData WeatherService::getInput()
{
    return {
        .outTemperature = 20.0,
        .sunlightIntensity = 1000.0
    };
}

} // namespace POLA::Services::Inputs

