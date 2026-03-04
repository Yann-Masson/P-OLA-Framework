/**
 * @file Main.cpp
 * @brief Entry point for the P-OLA smart thermostat simulator.
 *
 * Uses SimulationBuilder to assemble the simulation like Lego bricks:
 * walls, windows, heater, services, and AI model — each one optional.
 *
 * If no model is found, run the trainer first:
 *   P-OLA_Trainer --help
 */

#include <filesystem>
#include <iostream>
#include <thread>


#include <torch/torch.h>

#include "Common/DataTypes.hpp"
#include "Models/AIModel.hpp"
#include "Simulation/ProviderSetup.hpp"


#include "Interfaces/IClock.hpp"
#include "Interfaces/IConsumptionService.hpp"
#include "Interfaces/IInputService.hpp"
#include "Interfaces/ITemperatureFactor.hpp"
#include "Interfaces/IAIRecorder.hpp"


#include "Simulation/Room/Room.hpp"

using namespace POLA::Common;
using namespace POLA::Interfaces;
using namespace POLA::Models;
using namespace POLA::Simulation;

int main() {
  const std::string modelPath = "models/ai_model.pt";
  const std::string dataCsvPath =
      std::string(DATA_DIR) + "/data_home_1_scheduled_GPS.csv";

  // ---- Build the simulation using Lego bricks ----
  std::cout << "Initializing simulation provider..." << std::endl;

  auto provider = SimulationBuilder()
                      .setClock(900.0) // 1 real second = 15 simulated minutes
                      .setDataSource(dataCsvPath)
                      .setRoom(20.0) // Start at 20°C
                      // Rectangular room: 4 walls
                      .addWall(5.0, 2.5, 0.3, 0.6) // Wall 1 (5m wide)
                      .addWall(4.0, 2.5, 0.3, 0.6) // Wall 2 (4m wide)
                      .addWall(5.0, 2.5, 0.3, 0.6) // Wall 3 (opposite of 1)
                      .addWall(4.0, 2.5, 0.3, 0.6) // Wall 4 (opposite of 2)
                      // 1 window
                      .addWindow(2.0, 1.8, 0.5)
                      // 1 heater (2000W)
                      .addHeater(2000.0)
                      // AI model
                      .useAIModel(modelPath)
                      .build();

  std::cout << "Service Provider initialized with services:" << std::endl;
  std::cout << "Temperature factors registered:" << std::endl;
  for (const auto &factor : provider.getAll<ITemperatureFactor>()) {
    std::cout << " - " << typeid(*factor).name() << std::endl;
  }

  std::cout << "Simulation clock initialized at time: "
            << provider.get<IClock>()->getElapsedTime() << " seconds"
            << std::endl;
  std::cout << "Energy price service initialized with current price: $"
            << provider.get<IInputService<EnergyPriceData>>()
                   ->getInput()
                   .pricesPerKwh[0]
            << " per kWh" << std::endl;
  std::cout << "Weather service initialized with current temperature: "
            << provider.get<IInputService<WeatherData>>()
                   ->getInput()
                   .forecast[0]
                   .outdoorTemp
            << "°C" << std::endl;
  std::cout << "GPS service initialized with current location: ("
            << provider.get<IInputService<GPSData>>()->getInput().distanceKm
            << " km)" << std::endl;
  std::cout
      << "User preference service initialized with preferred temperature: "
      << provider.get<IInputService<UserPreferenceData>>()
             ->getInput()
             .maxTemperature
      << "°C" << std::endl;
  std::cout << "Consumption service initialized with total energy: "
            << provider.get<IConsumptionService>()->getTotalEnergyKWh()
            << " kWh and total cost: $"
            << provider.get<IConsumptionService>()->getTotalCost() << std::endl;

  auto room = provider.get<Room>();
  std::cout << "Room initialized with temperature: " << room->getTemperature()
            << "°C" << std::endl;

  // ---- Write AI records to CSV ----
  auto recorder = provider.get<POLA::Interfaces::IAIRecorder>();
  recorder->writeToCSV("ai_records.csv");
  std::cout << "AI records saved to ai_records.csv" << std::endl;

  return 0;
}
