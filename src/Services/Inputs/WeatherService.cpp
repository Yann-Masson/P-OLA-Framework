/**
 * @file WeatherService.cpp
 * @brief Implementation of the weather data service.
 */

#include "WeatherService.hpp"

using namespace POLA::Common;
using namespace POLA::Services::Inputs;
using namespace POLA::Interfaces;
using namespace POLA::Simulation;

WeatherData WeatherService::getInput()
{
    auto clock = _provider.get<IClock>();
    uint32_t currentTime = clock->getElapsedTimeSinceStart();
    auto dataManager = _provider.get<DataManager>();
    WeatherData weatherData;
    for (int i = 0; i < FORECAST_HOURS; i++) {
        DataPoint dp = dataManager->getDataPointForTime(currentTime + i * 3600);
        weatherData.forecast.push_back(WeatherDataPoint{dp.outdoor_temp, dp.light_level});
    }
    return weatherData;
}
