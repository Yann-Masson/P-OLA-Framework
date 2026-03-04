/**
 * @file SmartThermostatNoGPS.cpp
 * @brief GPS-free smart thermostat: builds AIStateNoGPS without GPS service
 * calls.
 */

#include <algorithm>

#include "Interfaces/IAIModelNoGPS.hpp"
#include "Interfaces/IClock.hpp"
#include "Interfaces/IInputService.hpp"

#include "SmartThermostatNoGPS.hpp"

#include "Common/AIStateNoGPS.hpp"
#include "Common/DataTypes.hpp"
#include "Simulation/TemperatureFactor/Heater.hpp"

using namespace POLA::Common;
using namespace POLA::Interfaces;
using namespace POLA::Simulation;

SmartThermostatNoGPS::SmartThermostatNoGPS(const forge::ProviderRef& provider)
    : _provider(provider)
{
}

void SmartThermostatNoGPS::reset() { _totalElapsedTime = 0; }

double SmartThermostatNoGPS::decide(const double currentTemp)
{
    const auto elapsedTime = _provider.get<IClock>()->getElapsedTime();
    _totalElapsedTime += elapsedTime;

    if (_totalElapsedTime < DECIDE_DELAY)
        return -1.0;

    _totalElapsedTime = 0;

    // Aggregate data — GPS intentionally omitted
    const auto energyPrice =
        _provider.get<IInputService<EnergyPriceData>>()->getInput();
    const auto weather = _provider.get<IInputService<WeatherData>>()->getInput();
    const auto userPref =
        _provider.get<IInputService<UserPreferenceData>>()->getInput();
    const auto userSchedule =
        _provider.get<IInputService<UserScheduleData>>()->getInput();

    AIStateNoGPS state{
        currentTemp,
        energyPrice.pricesPerKwh[0],
        weather,
        userPref,
        userSchedule
    };

    const auto aiModel = _provider.get<IAIModelNoGPS>();
    return aiModel->predict(state);
}

void SmartThermostatNoGPS::simulate(const double currentTemp)
{
    const double power = decide(currentTemp);
    if (power < 0)
        return;
    auto heater = _provider.get<TemperatureFactor::Heater>();
    heater->setPower(std::clamp(power, 0.0, 1.0));
}
