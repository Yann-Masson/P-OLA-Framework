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
    Window(const forge::ProviderRef& provider, double area, double uValue, double shgc);
    double simulate(double insideTemperature) override;

private:
    double _area;
    double _uValue; // Much higher than a wall (e.g., 1.8)
    double _shgc;   // Solar Heat Gain Coefficient (e.g., 0.5)
};

} // namespace POLA::Simulation::TemperatureFactor
