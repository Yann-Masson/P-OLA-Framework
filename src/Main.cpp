/**
 * @file Main.cpp
 * @brief Entry point for the P-OLA smart thermostat simulator.
 *
 * Loads a pre-trained PPO model (produced by P-OLA_Trainer) and runs
 * the simulation with all services and temperature factors.
 *
 * If no model is found, run the trainer first:
 *   P-OLA_Trainer --help
 */

#include <iostream>
#include <filesystem>
#include <thread>

#include <torch/torch.h>
#include <forge/provider_builder.hpp>

#include "Models/AIModel.hpp"
#include "Common/DataTypes.hpp"

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

using namespace POLA::Common;
using namespace POLA::Interfaces;
using namespace POLA::Models;
using namespace POLA::Services;
using namespace POLA::Services::Inputs;
using namespace POLA::Simulation;
using namespace POLA::Simulation::TemperatureFactor;

int main() {
    namespace fs = std::filesystem;

    const std::string modelPath = "models/ai_model.pt";

    if (!fs::exists(modelPath)) {
        std::cerr << "[ERROR] No trained model found at: " << modelPath << std::endl;
        std::cerr << "[INFO]  Train a model first: P-OLA_Trainer --help" << std::endl;
        return 1;
    }

    // Test the model standalone
    AIModel model(modelPath);

    constexpr AIState state {
        21.0,   // tempIn
        10.0,   // tempOut
        0.25,   // electricityPrice
        2.0,    // gpsDistance
        1.2,    // userVelocity
        22.0    // targetTemp
    };

    std::cout << "[AIModel] Predicted heater power: " << model.predict(state) << std::endl;

    // Simulation setup
    auto simulationClockService = std::make_shared<Clock>(900); // 1 real second = 15 minutes
    const std::string dataCsvPath = std::string(DATA_DIR) + "/data_home_1_scheduled.csv";
    auto dataManager = std::make_shared<DataManager>(dataCsvPath);

    // Configuration of the services
    const auto provider = forge::ProviderBuilder()
                              .addService<IClock>(simulationClockService)
                              .addService<IInputService<EnergyPriceData>, EnergyPriceService>()
                              .addService<IInputService<WeatherData>, WeatherService>()
                              .addService<IInputService<GPSData>, GPSService>()
                              .addService<IInputService<UserPreferenceData>, UserPreferenceService>()
                              .addService<IInputService<UserScheduleData>, UserScheduleService>()
                              .addService<IConsumptionService, ConsumptionService>()
                              .addService<ITemperatureFactor, Heater>()
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
                              .addService<ITemperatureFactor, Window>(
                                  std::function<std::shared_ptr<Window>(ProviderRef)>([](ProviderRef p) {
                                      return std::make_shared<Window>(p, 2.0, 1.8, 0.5);
                                  }))
                              .addService<IAIModel, AIModel>()
                              .addService<ISmartThermostat, SmartThermostat>()
                              .addService<Room>()
                              .addService<DataManager, DataManager>(dataManager)
                              .build();

    std::cout << "Service Provider initialized with services:" << std::endl;
    std::cout << "Temperature factors registered:" << std::endl;
    for (const auto &factor : provider.getAll<ITemperatureFactor>())
    {
        std::cout << " - " << typeid(*factor).name() << std::endl;
    }

    std::cout << "Simulation clock initialized at time: " << provider.get<IClock>()->getElapsedTime() << " seconds" << std::endl;
    std::cout << "Energy price service initialized with current price: $" << provider.get<IInputService<EnergyPriceData>>()->getInput().pricesPerKwh[0] << " per kWh" << std::endl;
    std::cout << "Weather service initialized with current temperature: " << provider.get<IInputService<WeatherData>>()->getInput().forecast[0].outdoorTemp << "°C" << std::endl;
    std::cout << "GPS service initialized with current location: (" << provider.get<IInputService<GPSData>>()->getInput().distanceKm << " km)" << std::endl;
    std::cout << "User preference service initialized with preferred temperature: " << provider.get<IInputService<UserPreferenceData>>()->getInput().maxTemperature << "°C" << std::endl;
    std::cout << "Consumption service initialized with total energy: " << provider.get<IConsumptionService>()->getTotalEnergyKWh() << " kWh and total cost: $" << provider.get<IConsumptionService>()->getTotalCost() << std::endl;

    auto room = provider.get<Room>();

    return 0;
}
