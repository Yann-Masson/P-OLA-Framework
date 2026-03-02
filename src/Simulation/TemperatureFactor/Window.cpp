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
#include "Interfaces/IInputService.hpp"

using namespace POLA::Common;
using namespace POLA::Interfaces;
using namespace POLA::Simulation::TemperatureFactor;

Window::Window(const forge::ProviderRef& provider, const double area, const double uValue, const double shgc)
    : ATemperatureFactor(provider), _area(area), _uValue(uValue), _shgc(shgc)
{
}

double Window::simulate(const double insideTemperature)
{
    const auto weatherService = _provider.get<IInputService<WeatherData>>();
    const auto [forecast] = weatherService->getInput();

    if (forecast.empty()) {
        // If we don't have weather data, we can't calculate heat loss/gain accurately.
        // For safety, we assume it's cold outside with no sunlight to avoid overheating the room.
        return _uValue * _area * insideTemperature; // Heat loss to a cold environment
    }

    const auto [outdoorTemp, sunlightLuxIntensity] = forecast[0];

    const auto heatLossConduction = _uValue * _area * (insideTemperature - outdoorTemp);
    const auto irradianceWattsPerSqMeter = sunlightLuxIntensity / 110.0; // Convert to Lux
    const auto solarHeatGain = _shgc * _area * irradianceWattsPerSqMeter;

    // Net Heat Loss: Conduction loss MINUS the free solar heating
    return heatLossConduction - solarHeatGain;
}
