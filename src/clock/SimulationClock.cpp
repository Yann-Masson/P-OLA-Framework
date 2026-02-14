#include "SimulationClock.hpp"

SimulationClock::SimulationClock()
{
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    _lastTime = now;
    _startTime = now;
}

void SimulationClock::simulate()
{
    auto now = std::chrono::steady_clock::now();
    _elapsedTime = std::chrono::duration<double, std::milli>(now - _lastTime).count();
    _lastTime = now;
}

double SimulationClock::elapsedTimeSinceStart() const
{
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - _startTime).count();
}

double SimulationClock::getElapsedTime() const
{
	return _elapsedTime;
}
