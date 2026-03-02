/**
 * @file Clock.hpp
 * @brief Simulation clock service tracking elapsed time between frames.
 */

#pragma once

#include <chrono>
#include "Interfaces/IClock.hpp"

namespace POLA::Services
{

    class Clock : public Interfaces::IClock
    {
    public:
        Clock(double timeMultiplier = 1.0);
        void simulate() override;
        void reset() override;
        [[nodiscard]] uint32_t getElapsedTimeSinceStart() const override;
        [[nodiscard]] uint32_t getElapsedTime() const override;

    private:
        std::chrono::steady_clock::time_point _startTime;
        std::chrono::steady_clock::time_point _lastTime;
        uint32_t _elapsedTime = 0;
        double _timeMultiplier; // Multiplier to speed up or slow down the simulation time
    };

} // namespace POLA::Services
