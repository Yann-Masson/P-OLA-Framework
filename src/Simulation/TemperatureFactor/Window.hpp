/**
 * @file Window.hpp
 * @brief Window temperature factor simulating heat loss through windows.
 */

#pragma once

#include "ATemperatureFactor.hpp"

namespace POLA::Simulation::TemperatureFactor {

class Window : public ATemperatureFactor
{
public:
    using ATemperatureFactor::ATemperatureFactor;
    double simulate(double insideTemperature) override;
};

} // namespace POLA::Simulation::TemperatureFactor
