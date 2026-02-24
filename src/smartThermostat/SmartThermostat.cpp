#include "SmartThermostat.hpp"

SmartThermostat::SmartThermostat(dic::ServiceProviderRef provider):
    _provider(provider)
{
}

double SmartThermostat::decide(double currentTemp)
{
    EnergyPriceData energyPrice = _provider.get<IInputService<EnergyPriceData>>()->getInput();
    WeatherData weather = _provider.get<IInputService<WeatherData>>()->getInput();
    UserPreferenceData userPref = _provider.get<IInputService<UserPreferenceData>>()->getInput();
    GPSData gps = _provider.get<IInputService<GPSData>>()->getInput();
    // TODO: construct the state object with the collected data
    // auto aiModel = _provider.get<AIModel>();
    // return aiModel->predict(/* state object */);
}
