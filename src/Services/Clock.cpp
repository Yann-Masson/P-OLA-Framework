/**
 * @file TrainingClock.cpp
 * @brief Implementation of the deterministic training clock.
 */

#include "Clock.hpp"

#include <iostream>

using namespace POLA::Services;

Clock::Clock(const double fixedDtSeconds)
    : _fixedDt(fixedDtSeconds)
{
}

void Clock::simulate()
{
    _totalTime += _fixedDt;
}

void Clock::reset() { _totalTime = 0; }

uint32_t Clock::getElapsedTimeSinceStart() const
{
    return static_cast<uint32_t>(_totalTime);
}

uint32_t Clock::getElapsedTime() const
{
    return static_cast<uint32_t>(_fixedDt);
}
