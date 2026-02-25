/**
 * @file Window.cpp
 * @brief Implementation of the window heat loss temperature factor.
 */

#include "Window.hpp"

namespace POLA::Simulation::TemperatureFactor {

double Window::simulate(double insideTemperature)
{
    // TODO: calculate the real value from the provider WeatherService (from the provider)
    return -5.0;
}

} // namespace POLA::Simulation::TemperatureFactor
