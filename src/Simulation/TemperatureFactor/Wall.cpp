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

using namespace POLA::Common;
using namespace POLA::Interfaces;

using namespace POLA::Simulation::TemperatureFactor;

Wall::Wall(const ProviderRef& provider, double width, double height, double uValue, double solarAbsorptance)
    : ATemperatureFactor(provider), _width(width), _height(height), _uValue(uValue), _solarAbsorptance(solarAbsorptance)
{
    _area = _width * _height;
}

double Wall::simulate(const double insideTemperature)
{
    const auto weatherService = _provider.get<IInputService<WeatherData>>();
    const auto [forecast] = weatherService->getInput();

    if (forecast.empty()) {
        // If we don't have weather data, we can't calculate heat loss/gain accurately.
        // For safety, we assume it's cold outside with no sunlight to avoid overheating the room.
        return _uValue * _area * insideTemperature; // Heat loss to a cold environment
    }

    const auto [outdoorTemp, sunlightLuxIntensity] = forecast[0]; // Use current weather conditions for this simulation step

    // 1. Calculate conductive/convective heat loss to the outside
    // Positive result means heat is escaping the room.
    const auto heatLossConduction = _uValue * _area * (insideTemperature - outdoorTemp);

    // 2. Calculate solar heat gain
    const auto irradianceWattsPerSqMeter = sunlightLuxIntensity / 110.0; // Convert Lux to W/m²

    const double wallSHGC = _solarAbsorptance * 0.35; // fraction that actually enters the room
    const auto solarHeatGain = wallSHGC * _area * irradianceWattsPerSqMeter;

    // 3. Calculate Net Heat Loss (Watts)
    // We subtract solar heat gain because it adds heat to the room, reducing net loss.
    const auto netHeatLoss = heatLossConduction - solarHeatGain;

    // Return the heat loss in Watts (Joules per second)
    return netHeatLoss;
}
