/**
 * @file TrainingEnvironment.cpp
 * @brief Implementation of the self-contained training environment.
 */

#include "TrainingEnvironment.hpp"

#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace POLA::Training;
using namespace POLA::Common;

TrainingEnvironment::TrainingEnvironment(const TrainingConfig& config, const uint32_t seed)
    : _config(config)
    , _rewardFn(config)
    , _rng(seed)
{
    _totalConductance = config.wallConductancePerUnit * config.numWalls
                      + config.windowConductance * config.numWindows;
}

AIState TrainingEnvironment::reset()
{
    // Randomize the episode scenario for diverse training experience
    std::uniform_real_distribution<double> tempOutDist(-10.0, 15.0);
    std::uniform_real_distribution<double> tempInDist(14.0, 24.0);
    std::uniform_real_distribution<double> targetDist(19.0, 24.0);
    std::uniform_real_distribution<double> distDist(5.0, 30.0);
    std::uniform_int_distribution<int>     returnStepDist(30, 180);
    std::uniform_real_distribution<double> speedDist(0.5, 1.5);       // km/min ≈ 30–90 km/h
    std::uniform_real_distribution<double> phaseDist(0.0, 2.0 * M_PI);
    std::uniform_real_distribution<double> priceBaseDist(0.08, 0.25);

    _step = 0;

    // Weather
    _tempOutBase = tempOutDist(_rng);
    _tempOut     = _tempOutBase;

    // Room
    _tempIn     = tempInDist(_rng);
    _targetTemp = targetDist(_rng);

    // GPS trajectory: user starts away, returns at a random time
    _initialDistance = distDist(_rng);
    _gpsDistance     = _initialDistance;
    _gpsVelocity    = 0.0;
    _returnStep      = returnStepDist(_rng);
    _returnSpeed     = speedDist(_rng);

    // Electricity pricing: sinusoidal with random phase
    _pricePhase = phaseDist(_rng);
    _priceBase  = priceBaseDist(_rng);
    _price      = computePrice();

    return getState();
}

std::tuple<AIState, double, bool> TrainingEnvironment::step(double heaterPower)
{
    heaterPower = std::clamp(heaterPower, 0.0, 1.0);

    const AIState currentState = getState();

    // ---- Room thermal dynamics (lumped-capacitance model) ----
    // Q_heater = power × P_max (watts)
    // Q_loss   = U_total × (T_in - T_out) (watts)
    // dT       = (Q_heater - Q_loss) × dt / C_thermal
    const double heatGainW = heaterPower * _config.maxHeaterPowerW;
    const double heatLossW = _totalConductance * (_tempIn - _tempOut);
    const double netHeatW  = heatGainW - heatLossW;
    _tempIn += netHeatW * _config.dt / _config.thermalCapacitance;

    // Clamp to physically reasonable bounds
    _tempIn = std::clamp(_tempIn, -10.0, 50.0);

    _step++;
    updateDynamics();

    const AIState nextState = getState();
    const bool done = (_step >= _config.episodeLength);

    // Compute multi-objective reward
    const double reward = _rewardFn.compute(currentState, heaterPower, nextState);

    return {nextState, reward, done};
}

AIState TrainingEnvironment::getState() const
{
    return {
        _tempIn,
        _tempOut,
        _price,
        _gpsDistance,
        _gpsVelocity,
        _targetTemp
    };
}

void TrainingEnvironment::updateDynamics()
{
    // ---- Weather: slow drift with daily sinusoidal variation ----
    std::normal_distribution<double> weatherNoise(0.0, 0.02);
    _tempOut = _tempOutBase
             + weatherNoise(_rng) * _step
             + 3.0 * std::sin(2.0 * M_PI * _step / 1440.0); // warmer at "noon"

    // ---- GPS trajectory ----
    if (_step < _returnStep) {
        // User is away (stationary at initial location)
        _gpsDistance = _initialDistance;
        _gpsVelocity = 0.0;
    } else {
        // User is driving home
        _gpsVelocity = _returnSpeed;
        _gpsDistance  = std::max(0.0, _initialDistance - _returnSpeed * (_step - _returnStep));
        if (_gpsDistance <= 0.0) {
            _gpsDistance  = 0.0;
            _gpsVelocity = 0.0; // Arrived: stationary at home
        }
    }

    // ---- Electricity price: sinusoidal time-of-day pattern ----
    _price = computePrice();
}

double TrainingEnvironment::computePrice() const
{
    // Convert step to hours with random phase offset
    const double timeOfDay = std::fmod(
        _step * _config.dt / 3600.0 + _pricePhase, 24.0);

    // Peak pricing during daytime, low at night
    const double sinComponent = _priceAmplitude * std::sin(2.0 * M_PI * timeOfDay / 24.0);
    return std::max(0.01, _priceBase + sinComponent);
}
