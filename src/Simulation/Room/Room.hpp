/**
 * @file Room.hpp
 * @brief Simulated room whose temperature is affected by registered temperature
 * factors.
 */

#pragma once

#include <forge/provider.hpp>

namespace POLA::Simulation {

constexpr double AIR_DENSITY = 1.225;        // kg/m^3
constexpr double AIR_SPECIFIC_HEAT = 1005.0; // J/(kg*C)
constexpr double BUILDING_THERMAL_MASS =
    500000.0; // J/K — thermal inertia of walls, furniture, structure

class Room {
public:
  explicit Room(const forge::ProviderRef &provider,
                double startingTemperature = 20.0);

  double getTemperature() const;
  void setTemperature(double temperature);
  void simulate();

private:
  double _indoorTemp;
  forge::ProviderRef _provider;
  double _volume;
};

} // namespace POLA::Simulation
