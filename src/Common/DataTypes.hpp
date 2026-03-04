/**
 * @file DataTypes.hpp
 * @brief Common data structures used across services for sensor and user data.
 */

#pragma once

#include <vector>

namespace POLA::Common {

struct EnergyPriceData {
  std::vector<double> pricesPerKwh; // Price forecast for the next 6 hours
};

struct WeatherDataPoint {
  double outdoorTemp;
  double sunlightLuxIntensity;
};

struct WeatherData {
  std::vector<WeatherDataPoint> forecast; // Forecast for the next 6 hours
};

struct UserPreferenceData {
  double minTemperature;
  double maxTemperature;
};

struct GPSData {
  double distanceKm;
  double velocityKmMin;
};

struct UserScheduleData {
  std::vector<bool> userPresent;
};

} // namespace POLA::Common
