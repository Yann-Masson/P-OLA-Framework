/**
 * @file ProviderSetup.hpp
 * @brief Helper functions for creating and configuring the service provider.
 */

#pragma once

#include <string>
#include <memory>
#include <forge/provider.hpp>

namespace POLA::Common {

/**
 * @brief Creates a fully configured service provider with all simulation services.
 * 
 * This function sets up:
 * - Clock service with configurable time scale
 * - All input services (GPS, Weather, Energy Price, User Preferences, User Schedule)
 * - Consumption tracking service
 * - Temperature factors (Walls, Windows, Heater)
 * - AI Model (if modelPath is provided, otherwise uses rule-based model)
 * - SmartThermostat controller
 * - Room simulation
 * - DataManager (if dataPath is provided)
 * 
 * @param timeScale Simulation time scale (e.g., 900 means 1 real second = 15 minutes)
 * @param dataCsvPath Path to the CSV data file (optional, defaults to data_home_1_scheduled.csv)
 * @param modelPath Path to the trained AI model (optional, if empty uses rule-based model)
 * @param startingRoomTemp Initial room temperature in Celsius (default: 20.0)
 * @return forge::Provider Configured service provider ready for simulation
 */
forge::Provider createSimulationProvider(
    double timeScale = 900.0,
    const std::string& dataCsvPath = "",
    const std::string& modelPath = "",
    double startingRoomTemp = 20.0
);

} // namespace POLA::Common
