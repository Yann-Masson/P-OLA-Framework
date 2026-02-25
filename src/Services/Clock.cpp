#include "Clock.hpp"

Clock::Clock()
{
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    _lastTime = now;
    _startTime = now;
}

void Clock::simulate()
{
    auto now = std::chrono::steady_clock::now();
    _elapsedTime = std::chrono::duration<double, std::milli>(now - _lastTime).count();
    _lastTime = now;
}

double Clock::elapsedTimeSinceStart() const
{
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - _startTime).count();
}

double Clock::getElapsedTime() const
{
	return _elapsedTime;
}
