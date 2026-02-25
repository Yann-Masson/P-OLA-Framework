/**
 * @file Wall.hpp
 * @brief Wall temperature factor simulating heat loss through walls.
 */

#pragma once

#include "ATemperatureFactor.hpp"

namespace POLA::Simulation::TemperatureFactor {

class Wall : public ATemperatureFactor
{
public:
    using ATemperatureFactor::ATemperatureFactor;
    double simulate(double insideTemperature) override;
};

} // namespace POLA::Simulation::TemperatureFactor
