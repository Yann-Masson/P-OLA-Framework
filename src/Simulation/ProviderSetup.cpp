/**
 * @file ProviderSetup.cpp
 * @brief Implementation of provider setup helper functions.
 */

#include "ProviderSetup.hpp"

#include <iostream>
#include <filesystem>
#include <forge/provider_builder.hpp>

#include "Services/Clock.hpp"
#include "Services/ConsumptionService.hpp"
#include "Services/Inputs/GPSService.hpp"
#include "Services/Inputs/WeatherService.hpp"
#include "Services/Inputs/EnergyPriceService.hpp"
#include "Services/Inputs/UserPreferenceService.hpp"
#include "Services/Inputs/UserScheduleService.hpp"

#include "Simulation/Room/Room.hpp"
#include "Simulation/TemperatureFactor/Wall.hpp"
#include "Simulation/TemperatureFactor/Heater.hpp"
#include "Simulation/TemperatureFactor/Window.hpp"
#include "Simulation/SmartThermostat/SmartThermostat.hpp"
#include "Simulation/DataManager/DataManager.hpp"

#include "Models/AIModel.hpp"
#include "Models/RuleBasedModel.hpp"

#include "Interfaces/IClock.hpp"
#include "Interfaces/IAIModel.hpp"

using namespace POLA::Common;
using namespace POLA::Interfaces;
using namespace POLA::Services;
using namespace POLA::Services::Inputs;
using namespace POLA::Simulation;
using namespace POLA::Simulation::TemperatureFactor;

namespace POLA::Common {

forge::Provider createSimulationProvider(
    const double timeScale,
    const std::string& dataCsvPath,
    const std::string& modelPath,
    const double startingRoomTemp
) {
    namespace fs = std::filesystem;

    // Create the clock service
    auto simulationClockService = std::make_shared<Clock>(timeScale);

    // Determine data path
    std::string dataPath = dataCsvPath;
    if (dataPath.empty()) {
        dataPath = std::string(DATA_DIR) + "/data_home_1_scheduled.csv";
    }

    // Create data manager
    auto dataManager = std::make_shared<DataManager>(dataPath);

    // Build the provider
    auto builder = forge::ProviderBuilder()
        .addService<IClock>(simulationClockService)
        .addService<IInputService<EnergyPriceData>, EnergyPriceService>()
        .addService<IInputService<WeatherData>, WeatherService>()
        .addService<IInputService<GPSData>, GPSService>()
        .addService<IInputService<UserPreferenceData>, UserPreferenceService>()
        .addService<IInputService<UserScheduleData>, UserScheduleService>()
        .addService<IConsumptionService, ConsumptionService>()
        .addService<ITemperatureFactor, Heater>()
        // Add 4 walls for the room (rectangular room)
        .addService<ITemperatureFactor, Wall>(
            std::function<std::shared_ptr<Wall>(ProviderRef)>([](ProviderRef p) {
                return std::make_shared<Wall>(p, 5.0, 2.5, 0.3, 0.6);
            }))
        .addService<ITemperatureFactor, Wall>(
            std::function<std::shared_ptr<Wall>(ProviderRef)>([](ProviderRef p) {
                return std::make_shared<Wall>(p, 4.0, 2.5, 0.3, 0.6);
            }))
        .addService<ITemperatureFactor, Wall>(
            std::function<std::shared_ptr<Wall>(ProviderRef)>([](ProviderRef p) {
                return std::make_shared<Wall>(p, 5.0, 2.5, 0.3, 0.6);
            }))
        .addService<ITemperatureFactor, Wall>(
            std::function<std::shared_ptr<Wall>(ProviderRef)>([](ProviderRef p) {
                return std::make_shared<Wall>(p, 4.0, 2.5, 0.3, 0.6);
            }))
        // Add window
        .addService<ITemperatureFactor, Window>(
            std::function<std::shared_ptr<Window>(ProviderRef)>([](ProviderRef p) {
                return std::make_shared<Window>(p, 2.0, 1.8, 0.5);
            }));

    // Add AI model or rule-based model
    if (!modelPath.empty() && fs::exists(modelPath)) {
        std::cout << "[ProviderSetup] Using AI model from: " << modelPath << std::endl;
        builder.addService<IAIModel, Models::AIModel>(
            std::function<std::shared_ptr<Models::AIModel>(ProviderRef)>(
                [modelPath](ProviderRef p) {
                    return std::make_shared<Models::AIModel>(p, modelPath);
                }));
    } else {
        std::cout << "[ProviderSetup] Using rule-based model (no model path provided or file not found)" << std::endl;
        builder.addService<IAIModel, Models::RuleBasedModel>();
    }

    // Add remaining services
    builder.addService<ISmartThermostat, SmartThermostat>()
           .addService<Room>(
               std::function<std::shared_ptr<Room>(ProviderRef)>(
                   [startingRoomTemp](ProviderRef p) {
                       return std::make_shared<Room>(p, startingRoomTemp);
                   }))
           .addService<DataManager, DataManager>(dataManager);

    return builder.build();
}

} // namespace POLA::Common
