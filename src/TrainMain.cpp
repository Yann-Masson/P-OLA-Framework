/**
 * @file TrainMain.cpp
 * @brief Entry point for PPO training of the smart thermostat AI model.
 *
 * Uses SimulationBuilder to assemble the training environment with the same
 * Lego bricks as the real simulator — ensuring training physics match
 * deployment.
 *
 * Usage:
 *   P-OLA_Trainer [options]
 *
 * Options:
 *   --timesteps N       Total training steps (default: 1000000)
 *   --lr RATE           Learning rate (default: 3e-4)
 *   --w-comfort W       Comfort penalty weight (default: 0.5)
 *   --w-economy W       Economy penalty weight (default: 0.3)
 *   --w-gps W           GPS arrival penalty weight (default: 0.2)
 *   --hidden-dim N      Hidden layer size (default: 64)
 *   --rollout-steps N   Steps per rollout (default: 2048)
 *   --epochs N          PPO epochs per update (default: 4)
 *   --output PATH       Model save path (default: models/ai_model.pt)
 *   --seed N            Random seed (default: 42)
 *   --data PATH         CSV data file (default: data_home_1_scheduled.csv)
 *   --help              Show this help message
 *
 * After training, the model is saved as a self-contained TorchScript file
 * that can be loaded directly by the P-OLA simulator.
 */

#include <iostream>
#include <string>

#include "Interfaces/IAIModel.hpp"
#include "Interfaces/IClock.hpp"
#include "Interfaces/IAIRecorder.hpp"
#include "Simulation/ProviderSetup.hpp"
#include "Simulation/Room/Room.hpp"
#include "Training/PPOTrainingAgent.hpp"
#include "Training/TrainingConfig.hpp"

using namespace POLA::Training;
using namespace POLA::Common;
using namespace POLA::Simulation;
using namespace POLA::Interfaces;

int main(const int argc, char* argv[])
{
    TrainingConfig config;
    uint32_t seed = 42;
    std::string dataCsvPath = std::string(DATA_DIR) + "/data_home_1_scheduled_GPS.csv";

    // Parse command-line arguments
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "--timesteps" && i + 1 < argc)
            config.totalTimesteps = std::stoi(argv[++i]);
        else if (arg == "--lr" && i + 1 < argc)
            config.learningRate = std::stod(argv[++i]);
        else if (arg == "--w-comfort" && i + 1 < argc)
            config.wComfort = std::stod(argv[++i]);
        else if (arg == "--w-economy" && i + 1 < argc)
            config.wEconomy = std::stod(argv[++i]);
        else if (arg == "--hidden-dim" && i + 1 < argc)
            config.hiddenDim = std::stoi(argv[++i]);
        else if (arg == "--rollout-steps" && i + 1 < argc)
            config.rolloutSteps = std::stoi(argv[++i]);
        else if (arg == "--epochs" && i + 1 < argc)
            config.numEpochs = std::stoi(argv[++i]);
        else if (arg == "--output" && i + 1 < argc)
            config.modelSavePath = argv[++i];
        else if (arg == "--seed" && i + 1 < argc)
            seed = static_cast<uint32_t>(std::stoul(argv[++i]));
        else if (arg == "--data" && i + 1 < argc)
            dataCsvPath = argv[++i];
        else if (arg == "--help")
        {
            std::cout
                << "P-OLA Smart Thermostat - PPO Trainer\n"
                << "====================================\n\n"
                << "Trains a reinforcement learning agent to control heater power\n"
                << "while optimizing for comfort, energy cost, and GPS-aware "
                "arrival.\n\n"
                << "Usage: " << argv[0] << " [options]\n\n"
                << "Options:\n"
                << "  --timesteps N       Total training steps       (default: "
                "1000000)\n"
                << "  --lr RATE           Learning rate              (default: "
                "3e-4)\n"
                << "  --w-comfort W       Comfort penalty weight     (default: 0.5)\n"
                << "  --w-economy W       Economy penalty weight     (default: 0.3)\n"
                << "  --w-gps W           GPS arrival penalty weight (default: 0.2)\n"
                << "  --hidden-dim N      Hidden layer size          (default: 64)\n"
                << "  --rollout-steps N   Steps per rollout          (default: "
                "2048)\n"
                << "  --epochs N          PPO epochs per update      (default: 4)\n"
                << "  --output PATH       Model output path          (default: "
                "models/ai_model.pt)\n"
                << "  --seed N            Random seed                (default: 42)\n"
                << "  --data PATH         CSV data file              (default: "
                "data_home_1_scheduled.csv)\n"
                << "  --help              Show this help message\n\n"
                << "Example:\n"
                << "  " << argv[0]
                << " --timesteps 500000 --w-economy 0.4 --output "
                "models/eco_model.pt\n";
            return 0;
        }
        else
        {
            std::cerr << "Unknown argument: " << arg << " (use --help for options)"
                << std::endl;
            return 1;
        }
    }

    std::cout << "========================================" << std::endl;
    std::cout << "P-OLA Trainer - Full Simulation Mode" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Creating simulation environment..." << std::endl;

    // ---- Build the training environment using the same Lego bricks ----
    auto provider =
        SimulationBuilder()
        .useTrainingClock(60.0) // Fixed 60s per step (deterministic)
        .setDataSource(dataCsvPath)
        .setRoom(20.0)
        // Same room layout as the real simulator
        .addWall(5.0, 2.5, 0.3, 0.6)
        .addWall(4.0, 2.5, 0.3, 0.6)
        .addWall(5.0, 2.5, 0.3, 0.6)
        .addWall(4.0, 2.5, 0.3, 0.6)
        .addWindow(2.0, 1.8, 0.5)
        .addHeater(2000.0)
        // Training agent: The PPOTrainingAgent implements IAIModel and
        // handles all RL internally when predict() is called.
        .useTrainingAgent(config)
        .build();

    std::cout << "Simulation provider created successfully" << std::endl;
    std::cout << "Starting training simulation loop..." << std::endl;

    // ---- The Inverse RL simulation loop ----
    // The PPOTrainingAgent is injected as IAIModel and handles all training
    // internally. The Room calls the SmartThermostat, which calls predict()
    // on the agent — that's where the PPO magic happens.
    const auto room = provider.get<Room>();
    const auto clock = provider.get<IClock>();

    std::cout << "Running for " << config.totalTimesteps << " total timesteps.\n";

    for (int step = 0; step < config.totalTimesteps; ++step)
    {
        clock->simulate(); // Advance time
        room->simulate(); // Simulate room: calls thermostat, which calls predict()

        std::cout << "\r[TrainMain] Room temp: " << room->getTemperature()
            << "C | Step: " << step + 1 << "/" << config.totalTimesteps
            << std::flush;
    }

    std::cout << "\n[TrainMain] Training complete!" << std::endl;

    auto recorder = provider.get<POLA::Interfaces::IAIRecorder>();
    recorder->writeToCSV("ai_records_training.csv");
    std::cout << "AI records saved to ai_records_training.csv" << std::endl;

    return 0;
}
