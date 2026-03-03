/**
 * @file SmartThermostat.cpp
 * @brief Implementation of the smart thermostat controller.
 */

#include <algorithm>

#include "Interfaces/IAIModel.hpp"
#include "Interfaces/IClock.hpp"
#include "Interfaces/IInputService.hpp"

#include "SmartThermostat.hpp"

#include <iostream>

#include "Common/DataTypes.hpp"
#include "Simulation/TemperatureFactor/Heater.hpp"

using namespace POLA::Common;
using namespace POLA::Interfaces;

using namespace POLA::Simulation;

SmartThermostat::SmartThermostat(const forge::ProviderRef& provider):
    _provider(provider)
{
}

double SmartThermostat::decide(const double currentTemp)
{
    auto elapsedTime = _provider.get<IClock>()->getElapsedTime();

    _totalElapsedTime += elapsedTime;

    if (_totalElapsedTime < DECIDE_DELAY) {
        return -1.0;
    }

    _totalElapsedTime = 0; // reset timer after decision

    // Aggregation of data from input services
    const auto energyPrice = _provider.get<IInputService<EnergyPriceData>>()->getInput();
    const auto weather = _provider.get<IInputService<WeatherData>>()->getInput();
    const auto userPref = _provider.get<IInputService<UserPreferenceData>>()->getInput();
    const auto gps = _provider.get<IInputService<GPSData>>()->getInput();

    AIState state{
        currentTemp,
        weather.forecast[0].outdoorTemp,
        energyPrice.pricesPerKwh[0],
        gps.distanceKm,
        gps.velocityKmMin,
        (userPref.minTemperature + userPref.maxTemperature) / 2.0
    };

    const auto aiModel = _provider.get<IAIModel>();
    return aiModel->predict(state);
}

void SmartThermostat::simulate(const double currentTemp)
{
    double power = decide(currentTemp);
    if (power < 0) {
        return;
    }
    auto heater = _provider.get<TemperatureFactor::Heater>();
    heater->setPower(std::clamp(power, 0.0, 1.0));
}
