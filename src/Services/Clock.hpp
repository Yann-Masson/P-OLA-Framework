/**
 * @file Clock.hpp
 * @brief Simulation clock service tracking elapsed time between frames.
 */

#pragma once

#include <chrono>
#include "Interfaces/IClock.hpp"

namespace POLA::Services {

class Clock : public Interfaces::IClock
{
public:
    Clock();
    void simulate() override;
    [[nodiscard]] double getElapsedTimeSinceStart() const override;
    [[nodiscard]] double getElapsedTime() const override;

private:
    std::chrono::steady_clock::time_point _startTime;
    std::chrono::steady_clock::time_point _lastTime;
    double _elapsedTime = 0.0;
};

} // namespace POLA::Services

