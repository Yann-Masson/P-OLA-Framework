/**
 * @file TrainingClock.cpp
 * @brief Implementation of the deterministic training clock.
 */

#include "TrainingClock.hpp"

#include <iostream>

using namespace POLA::Services;

TrainingClock::TrainingClock(const double fixedDtSeconds)
    : _fixedDt(fixedDtSeconds)
{
}

void TrainingClock::simulate()
{
    _totalTime += _fixedDt;
}

void TrainingClock::reset() { _totalTime = 0; }

uint32_t TrainingClock::getElapsedTimeSinceStart() const
{
    return static_cast<uint32_t>(_totalTime);
}

uint32_t TrainingClock::getElapsedTime() const
{
    return static_cast<uint32_t>(_fixedDt);
}
