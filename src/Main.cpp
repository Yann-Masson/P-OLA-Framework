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

int main()
{
    const std::string modelPath = "models/ai_model.pt";
    const std::string dataCsvPath = std::string(DATA_DIR) + "/data_home_1_scheduled_GPS.csv";

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

    std::cout << "Simulation provider created successfully" << std::endl;
    std::cout << "Starting training simulation loop..." << std::endl;

    const auto room = provider.get<Room>();
    const auto clock = provider.get<IClock>();

    for (int step = 0; step < 1000000; ++step)
    {
        clock->simulate(); // Advance time
        room->simulate(); // Simulate room: calls thermostat, which calls predict()

        std::cout << "\r[TrainMain] Room temp: " << room->getTemperature()
            << "C | Step: " << step + 1 << "/" << 1000000
            << std::flush;
    }

    std::cout << "\n[TrainMain] Training complete!" << std::endl;

    // ---- Write AI records to CSV ----
    auto recorder = provider.get<POLA::Interfaces::IAIRecorder>();
    recorder->writeToCSV("ai_records.csv");
    std::cout << "AI records saved to ai_records.csv" << std::endl;

    return 0;
}
