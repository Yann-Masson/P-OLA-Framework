/**
 * @file Wall.cpp
 * @brief Implementation of the wall heat loss temperature factor.
 */

#include "Wall.hpp"

using namespace POLA::Simulation::TemperatureFactor;

double Wall::simulate(double insideTemperature)
{
    // TODO: calculate the real value from the provider WeatherService (from the provider)
    return -2.0;
}

