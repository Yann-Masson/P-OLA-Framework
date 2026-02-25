/**
 * @file Clock.cpp
 * @brief Implementation of the simulation clock service.
 */

#include "Clock.hpp"

using namespace POLA::Services;

Clock::Clock()
{
    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    _lastTime = now;
    _startTime = now;
}

void Clock::simulate()
{
    const auto now = std::chrono::steady_clock::now();
    _elapsedTime = std::chrono::duration<double, std::milli>(now - _lastTime).count();
    _lastTime = now;
}

double Clock::getElapsedTimeSinceStart() const
{
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - _startTime).count();
}

double Clock::getElapsedTime() const
{
    return _elapsedTime;
}
