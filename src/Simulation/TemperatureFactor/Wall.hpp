/**
 * @file Wall.hpp
 * @brief Wall temperature factor simulating heat loss through walls.
 */

#pragma once

#include "ATemperatureFactor.hpp"

using namespace forge;

namespace POLA::Simulation::TemperatureFactor
{
    class Wall : public ATemperatureFactor
    {
    public:
        Wall(const ProviderRef& provider, double width, double height, double uValue, double solarAbsorptance);
        double simulate(double insideTemperature) override;

    protected:
        double _width;
        double _height;

        friend class Room; // Allow Room to access protected members for simulation purposes
    private:
        double _area;
        double _uValue; // e.g., 0.3 W/(m^2*K) for a well-insulated wall
        double _solarAbsorptance; // e.g., 0.6 (60% of solar energy absorbed)
    };
} // namespace POLA::Simulation::TemperatureFactor
