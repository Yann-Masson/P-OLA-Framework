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
    const double dtMs = _provider.get<Interfaces::IClock>()->getElapsedTime();
    const double dtSec = dtMs / 1000.0;
    if (dtSec <= 0.0) return 0.0;

    const double outsideTemp =
        _provider.get<Interfaces::IInputService<Common::WeatherData>>()->getInput().outTemperature;

    constexpr double wallConductance = 3.75;        // W/K (one wall segment)
    constexpr double thermalCapacitance = 500000.0;  // J/K

    const double heatLossW = wallConductance * (insideTemperature - outsideTemp);
    return -heatLossW * dtSec / thermalCapacitance;
}

