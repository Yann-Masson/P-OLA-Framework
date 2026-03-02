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

#include "Models/AIModel.hpp"
#include "Common/DataTypes.hpp"
#include "Simulation/ProviderSetup.hpp"

#include "Interfaces/IClock.hpp"
#include "Interfaces/IInputService.hpp"
#include "Interfaces/IConsumptionService.hpp"
#include "Interfaces/ITemperatureFactor.hpp"

#include "Simulation/Room/Room.hpp"

using namespace POLA::Common;
using namespace POLA::Interfaces;
using namespace POLA::Models;
using namespace POLA::Simulation;

int main() {
    const std::string modelPath = "models/ai_model.pt";

    // Create the simulation provider using the helper function
    std::cout << "Initializing simulation provider..." << std::endl;
    const std::string dataCsvPath = std::string(DATA_DIR) + "/data_home_1_scheduled_GPS.csv";
    auto provider = createSimulationProvider(
        900.0,        // timeScale: 1 real second = 15 minutes
        dataCsvPath,  // CSV data path
        modelPath,    // AI model path
        20.0          // Starting room temperature
    );

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
    std::cout << "Room initialized with temperature: " << room->getTemperature() << "°C" << std::endl;

    return 0;
}
