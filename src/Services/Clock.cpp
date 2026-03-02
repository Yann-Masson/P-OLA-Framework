/**
 * @file Clock.cpp
 * @brief Implementation of the simulation clock service.
 */

#include "Clock.hpp"

using namespace POLA::Services;

Clock::Clock(double timeMultiplier)
    : _timeMultiplier(timeMultiplier)
{
    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    _lastTime = now;
    _startTime = now;
}

void Clock::simulate()
{
    const auto now = std::chrono::steady_clock::now();
    _elapsedTime = std::chrono::duration<double>(now - _lastTime).count();
    _lastTime = now;
}

void Clock::reset()
{
    const auto now = std::chrono::steady_clock::now();
    _lastTime = now;
    _startTime = now;
    _elapsedTime = 0;
}

uint32_t Clock::getElapsedTimeSinceStart() const
{
    return static_cast<uint32_t>(std::chrono::duration<double>(std::chrono::steady_clock::now() - _startTime).count() * _timeMultiplier);
}

uint32_t Clock::getElapsedTime() const
{
    return static_cast<uint32_t>(_elapsedTime * _timeMultiplier);
}
