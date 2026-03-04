/**
 * @file AIState.hpp
 * @brief Definition of the AIState struct representing the input state for the
 * AI model.
 */

#pragma once

#include "DataTypes.hpp"

namespace POLA::Common
{
    struct AIState
    {
        // ---- Current room sensor readings ----
        double tempIn; ///< Current indoor temperature (°C)
        double electricityPrice; ///< Current electricity price ($/kWh)

        // ---- User position (GPS) ----
        double userDistanceKm; ///< Distance from home (km)
        double userVelocityKmMin; ///< User speed (km/min)

        // ---- Weather forecast (next N hours) ----
        WeatherData weather; ///< Full weather forecast (outdoor temp + sunlight per hour)

        // ---- User comfort preferences ----
        UserPreferenceData userPreferences; ///< Min/max desired temperature

        // ---- Occupancy schedule (next 24 hours) ----
        UserScheduleData userSchedule; ///< Whether the user will be present each hour
    };
} // namespace POLA::Common
