/**
 * @file DataTypes.hpp
 * @brief Common data structures used across services for sensor and user data.
 */

#pragma once

namespace POLA::Common {

struct EnergyPriceData {
    double pricePerKWh;
};

struct WeatherDataPoint {
    double outTemperature;
    double sunlightIntensity; // in lux
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

