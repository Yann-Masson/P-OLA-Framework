/**
 * @file SmartThermostat.cpp
 * @brief Implementation of the smart thermostat controller.
 */

#include "Interfaces/IAIModel.hpp"
#include "Interfaces/IClock.hpp"
#include "Interfaces/IInputService.hpp"

#include "SmartThermostat.hpp"
#include "Common/DataTypes.hpp"
#include "Simulation/TemperatureFactor/Heater.hpp"

using namespace POLA::Common;
using namespace POLA::Interfaces;

namespace POLA::Simulation {

SmartThermostat::SmartThermostat(const forge::ProviderRef& provider):
    _provider(provider)
{
}

double SmartThermostat::decide(const double currentTemp) const
{
    auto clock = _provider.get<IClock>()->getElapsedTime();
    if (clock < DECIDE_DELAY) {
        return -1.0;
    }

    // Aggregation of data from input services
    const auto energyPrice = _provider.get<IInputService<EnergyPriceData>>()->getInput();
    const auto weather = _provider.get<IInputService<WeatherData>>()->getInput();
    const auto userPref = _provider.get<IInputService<UserPreferenceData>>()->getInput();
    const auto gps = _provider.get<IInputService<GPSData>>()->getInput();

    AIState state{
        currentTemp,
        weather.outTemperature,
        energyPrice.pricePerKWh,
        gps.distanceKm,
        gps.velocityKmMin,
        (userPref.minTemperature + userPref.maxTemperature) / 2.0
    };

    const auto aiModel = _provider.get<IAIModel>();
    return aiModel->predict(state);
}

void SmartThermostat::simulate(const double currentTemp)
{
    double wantedTemp = decide(currentTemp);
    if (wantedTemp < 0) {
        return;
    }
    auto heater = _provider.get<TemperatureFactor::Heater>();
    heater->setWantedTemperature(wantedTemp);
}

} // namespace POLA::Simulation
