/**
 * @file Wall.cpp
 * @brief Implementation of the wall heat loss temperature factor.
 */

#include "Wall.hpp"

namespace POLA::Simulation::TemperatureFactor {

double Wall::simulate(double insideTemperature)
{
    // TODO: calculate the real value from the provider WeatherService (from the provider)
    return -2.0;
}

} // namespace POLA::Simulation::TemperatureFactor
