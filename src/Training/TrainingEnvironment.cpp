/**
 * @file TrainingEnvironment.cpp
 * @brief Implementation of the full-simulation training environment.
 */

#include "TrainingEnvironment.hpp"

#include <cmath>
#include <algorithm>
#include <iostream>

#include "Interfaces/IClock.hpp"
#include "Interfaces/IInputService.hpp"
#include "Common/DataTypes.hpp"
#include "Simulation/Room/Room.hpp"
#include "Simulation/TemperatureFactor/Heater.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace POLA::Training;
using namespace POLA::Common;
using namespace POLA::Interfaces;
using namespace POLA::Simulation;
using namespace POLA::Simulation::TemperatureFactor;

TrainingEnvironment::TrainingEnvironment(
    forge::Provider provider,
    const TrainingConfig& config,
    const uint32_t seed
)
    : _provider(std::move(provider))
    , _config(config)
    , _rewardFn(config)
    , _rng(seed)
{
    std::cout << "[TrainingEnvironment] Initialized with full simulation provider" << std::endl;
}

AIState TrainingEnvironment::reset()
{
    _step = 0;

    // Reset the clock to zero
    auto clock = _provider.get<IClock>();
    clock->reset();

    // Get the room and set a random initial temperature
    auto room = _provider.get<Room>();
    std::uniform_real_distribution<double> tempInDist(14.0, 24.0);
    // Note: We would need a setter in Room to change the temperature
    // For now, the room will start at its default temperature

    // Reset heater to off
    auto heater = _provider.get<Heater>();
    heater->setPower(0.0);

    std::cout << "[TrainingEnvironment] Episode reset. Initial room temp: "
              << room->getTemperature() << "°C" << std::endl;

    return getState();
}

std::tuple<AIState, double, bool> TrainingEnvironment::step(double heaterPower)
{
    heaterPower = std::clamp(heaterPower, 0.0, 1.0);

    const AIState currentState = getState();

    // Set heater power for this step
    auto heater = _provider.get<Heater>();
    heater->setPower(heaterPower);

    // Advance the simulation by one time step
    auto clock = _provider.get<IClock>();
    clock->simulate();

    // Simulate the room thermal dynamics
    auto room = _provider.get<Room>();
    room->simulate();

    _step++;

    const AIState nextState = getState();
    const bool done = (_step >= _config.episodeLength);

    // Compute multi-objective reward
    const double reward = _rewardFn.compute(currentState, heaterPower, nextState);

    return {nextState, reward, done};
}

AIState TrainingEnvironment::getState() const
{
    // Get current state from services
    auto room = _provider.get<Room>();
    auto weatherService = _provider.get<IInputService<WeatherData>>();
    auto energyPriceService = _provider.get<IInputService<EnergyPriceData>>();
    auto gpsService = _provider.get<IInputService<GPSData>>();
    auto userPrefService = _provider.get<IInputService<UserPreferenceData>>();

    const auto weather = weatherService->getInput();
    const auto energyPrice = energyPriceService->getInput();
    const auto gps = gpsService->getInput();
    const auto userPref = userPrefService->getInput();

    const double tempIn = room->getTemperature();
    const double tempOut = weather.forecast[0].outdoorTemp;
    const double price = energyPrice.pricesPerKwh[0];
    const double gpsDistance = gps.distanceKm;
    const double gpsVelocity = gps.velocityKmMin;
    const double targetTemp = (userPref.minTemperature + userPref.maxTemperature) / 2.0;

    return {
        tempIn,
        tempOut,
        price,
        gpsDistance,
        gpsVelocity,
        targetTemp
    };
}
