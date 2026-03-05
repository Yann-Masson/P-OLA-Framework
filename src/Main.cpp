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
#include "Interfaces/IUserComfortService.hpp"


#include "Simulation/Room/Room.hpp"

using namespace POLA::Common;
using namespace POLA::Interfaces;
using namespace POLA::Models;
using namespace POLA::Simulation;

int main()
{
    const std::string modelPath = "models/ai_model.pt";
    const std::string dataCsvPath = std::string(DATA_DIR) + "/data_home_1_scheduled_GPS.csv";

    // ---- Build the simulation using Lego bricks ----
    std::cout << "Initializing simulation provider..." << std::endl;

    auto provider = SimulationBuilder()
                    .setClock(60.0) // Fixed 60s per step
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
                    // .useAIModel(modelPath)
                    .useRuleBasedModel()
                    .build();

    std::cout << "Simulation provider created successfully" << std::endl;
    std::cout << "Starting training simulation loop..." << std::endl;

    const auto room = provider.get<Room>();
    const auto clock = provider.get<IClock>();
    const auto userComfortService = provider.get<IUserComfortService>();

    // 10 000 Mean from november to march
    for (int step = 0; step < 10000; ++step)
    {
        clock->simulate();
        room->simulate();
        userComfortService->recordComfort(room->getTemperature());

        std::cout << "\r[TrainMain] Room temp: " << room->getTemperature()
            << "C | Step: " << step + 1 << "/" << 1000000
            << std::flush;
    }

    std::cout << "\n[Main] Simulation complete!" << std::endl;

    const auto comsumptionService = provider.get<IConsumptionService>();

    std::cout << "Final room temperature: " << room->getTemperature() << " °C" << std::endl;
    std::cout << "Average user comfort: " << userComfortService->getUserComfort() << "%" << std::endl;
    std::cout << "Total energy consumed: " << comsumptionService->getTotalEnergyKWh() << " kWh" << std::endl;
    std::cout << "Total Cost energy: " << comsumptionService->getTotalCost() << " currency units" << std::endl;

    return 0;
}
