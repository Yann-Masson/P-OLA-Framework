#include "SmartThermostat.hpp"

SmartThermostat::SmartThermostat(dic::ServiceProviderRef provider):
    _provider(provider)
{
}

double SmartThermostat::decide(double currentTemp)
{
    auto clock = _provider.get<IClock>()->getElapsedTime();
    if (clock < DECIDE_DELAY) {
        return -1.0;
    }

    // Aggregation of data from input services
    EnergyPriceData energyPrice = _provider.get<IInputService<EnergyPriceData>>()->getInput();
    WeatherData weather = _provider.get<IInputService<WeatherData>>()->getInput();
    UserPreferenceData userPref = _provider.get<IInputService<UserPreferenceData>>()->getInput();
    GPSData gps = _provider.get<IInputService<GPSData>>()->getInput();
    State state{
        currentTemp,
        weather.outTemperature,
        energyPrice.pricePerKWh,
        gps.distanceKm,
        gps.velocityKmMin,
        (userPref.minTemperature + userPref.maxTemperature) / 2.0
    };
    auto aiModel = _provider.get<IAIModel>();
    return aiModel->predict(state);
}

void SmartThermostat::simulate(double currentTemp)
{
    double wantedTemp = decide(currentTemp);
    if (wantedTemp < 0) {
        return;
    }
    auto heater = _provider.get<Heater>();
    heater->setWantedTemperature(wantedTemp);
}
