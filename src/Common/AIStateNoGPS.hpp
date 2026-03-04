/**
 * @file AIStateNoGPS.hpp
 * @brief GPS-free state struct for the no-GPS thermostat model.
 *
 * Mirrors AIState exactly, but omits userDistanceKm and userVelocityKmMin.
 * Results in a 40-dimensional input vector (vs. 42 for the GPS model):
 *
 *   tempIn(1) + electricityPrice(1) +
 *   weather.forecast(6×2 = 12)     +
 *   userPreferences(2)              +
 *   userSchedule(24)
 *   = 40 total features
 */

#pragma once

#include "DataTypes.hpp"

namespace POLA::Common {
struct AIStateNoGPS {
  // ---- Current room sensor readings ----
  double tempIn;           ///< Current indoor temperature (°C)
  double electricityPrice; ///< Current electricity price ($/kWh)

  // (GPS omitted intentionally)

  // ---- Weather forecast (next N hours) ----
  WeatherData
      weather; ///< Full weather forecast (outdoor temp + sunlight per hour)

  // ---- User comfort preferences ----
  UserPreferenceData userPreferences; ///< Min/max desired temperature

  // ---- Occupancy schedule (next 24 hours) ----
  UserScheduleData userSchedule; ///< Whether the user will be present each hour
};
} // namespace POLA::Common
