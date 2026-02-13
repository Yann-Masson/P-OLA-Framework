/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** SimulationClock
*/

#pragma once
#include "IClock.hpp"
#include <chrono>

class SimulationClock : public IClock
{
public:
    SimulationClock();
    void simulate() override;
    double elapsedTimeSinceStart() const override;
    double getElapsedTime() const override;

protected:
private:
    std::chrono::steady_clock::time_point _startTime;
    std::chrono::steady_clock::time_point _lastTime;
    double _elapsedTime = 0.0;
};
