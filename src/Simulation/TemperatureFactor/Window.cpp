/**
 * @file Window.cpp
 * @brief Implementation of the window heat loss temperature factor.
 *
 * Windows contribute both heat loss and solar gain:
 *   dT = (-conductance × (T_in - T_out) + solar_gain) × dt / C_thermal
 *
 *   conductance = U_value × area ≈ 1.5 W/(m²·K) × 5 m² = 7.5 W/K
 *   solar_gain  ≈ 0.003 × sunlight_intensity (simplified model)
 */

#include "Window.hpp"

#include "Common/DataTypes.hpp"
#include "Interfaces/IClock.hpp"
#include "Interfaces/IInputService.hpp"

using namespace POLA::Simulation::TemperatureFactor;

double Window::simulate(double insideTemperature)
{
    // TODO: calculate the real value from the provider WeatherService (from the provider)
    return -5.0;
}
