/**
 * @file Wall.cpp
 * @brief Implementation of the wall heat loss temperature factor.
 *
 * Each wall contributes heat loss proportional to (T_in - T_out):
 *   dT = -conductance × (T_in - T_out) × dt / C_thermal
 *
 *   conductance = U_value × area ≈ 0.3 W/(m²·K) × 12.5 m² = 3.75 W/K
 *   C_thermal   = 500,000 J/K (shared room thermal mass)
 */

#include "Wall.hpp"

#include "Common/DataTypes.hpp"
#include "Interfaces/IClock.hpp"
#include "Interfaces/IInputService.hpp"

using namespace POLA::Simulation::TemperatureFactor;

double Wall::simulate(double insideTemperature)
{
    // TODO: calculate the real value from the provider WeatherService (from the provider)
    return -2.0;
}
