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
    const double dtMs = _provider.get<Interfaces::IClock>()->getElapsedTime();
    const double dtSec = dtMs / 1000.0;
    if (dtSec <= 0.0) return 0.0;

    const auto weather =
        _provider.get<Interfaces::IInputService<Common::WeatherData>>()->getInput();

    constexpr double windowConductance = 7.5;        // W/K
    constexpr double thermalCapacitance = 500000.0;   // J/K
    constexpr double solarGainFactor = 0.003;         // Simplified solar transmittance

    const double heatLossW = windowConductance * (insideTemperature - weather.outTemperature);
    const double solarGainW = solarGainFactor * weather.sunlightIntensity;

    return (-heatLossW + solarGainW) * dtSec / thermalCapacitance;
}

