/**
 * @file TrainMainNoGPS.cpp
 * @brief Entry point for PPO training of the GPS-free smart thermostat model.
 *
 * Identical to TrainMain.cpp, except:
 *   - Uses PPOTrainingAgentNoGPS (4-feature state, no GPS inputs)
 *   - Default output path: models/ai_model_no_gps.pt
 *   - No --w-gps option (GPS penalty is absent from the reward function)
 *
 * Usage:
 *   P-OLA_Trainer_NoGPS [options]
 *
 * Options:
 *   --timesteps N       Total training steps (default: 1000000)
 *   --lr RATE           Learning rate (default: 3e-4)
 *   --w-comfort W       Comfort penalty weight (default: 0.8)
 *   --w-economy W       Economy penalty weight (default: 0.2)
 *   --hidden-dim N      Hidden layer size (default: 64)
 *   --rollout-steps N   Steps per rollout (default: 2048)
 *   --epochs N          PPO epochs per update (default: 4)
 *   --output PATH       Model save path (default: models/ai_model_no_gps.pt)
 *   --seed N            Random seed (default: 42)
 *   --data PATH         CSV data file (default: data_home_1_scheduled_GPS.csv)
 *   --help              Show this help message
 */

#include <iostream>
#include <string>

#include "Interfaces/IAIRecorder.hpp"
#include "Interfaces/IClock.hpp"
#include "Simulation/ProviderSetup.hpp"
#include "Simulation/Room/Room.hpp"
#include "Training/OLA/OLATrainingAgent.hpp"
#include "Training/TrainingConfig.hpp"
#include "Interfaces/IConsumptionService.hpp"
#include "Interfaces/IUserComfortService.hpp"

using namespace POLA::Common;
using namespace POLA::Simulation;
using namespace POLA::Interfaces;

int main(const int argc, char *argv[])
{
    POLA::Training::TrainingConfig config;
    config.modelSavePath = "models/ai_model_no_gps.pt";

    std::string dataCsvPath = std::string(DATA_DIR) + "/data_home_1_scheduled_GPS.csv";

    std::string dataDir = std::string(DATA_DIR);

    std::string defaultDataPath = dataDir + "/data_home_1_scheduled_GPS.csv";
    std::string outputDataPath = "ai_records_training_no_gps.csv";

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
        else if (arg == "--data" && i + 1 < argc)
            dataCsvPath = argv[++i];
        else if (arg == "--output-data" && i + 1 < argc)
            outputDataPath = argv[++i];
        else if (arg == "--no-save")
            config.saveEnabled = false;
        else if (arg == "--help")
        {
            std::cout
                << "P-OLA Smart Thermostat - PPO Trainer (No GPS)\n"
                << "=============================================\n\n"
                << "Trains a reinforcement learning agent to control heater "
                   "power\n"
                << "using only thermal and economic signals (no GPS data).\n\n"
                << "Usage: " << argv[0] << " [options]\n\n"
                << "Options:\n"
                << "  --timesteps N       Total training steps       (default: "
                   "1000000)\n"
                << "  --lr RATE           Learning rate              (default: "
                   "3e-4)\n"
                << "  --w-comfort W       Comfort penalty weight     (default: 0.8)\n"
                << "  --w-economy W       Economy penalty weight     (default: 0.2)\n"
                << "  --hidden-dim N      Hidden layer size          (default: 64)\n"
                << "  --rollout-steps N   Steps per rollout          (default: "
                   "2048)\n"
                << "  --epochs N          PPO epochs per update      (default: 4)\n"
                << "  --output PATH       Model output path          (default: "
                   "models/ai_model_no_gps.pt)\n"
                << "  --data PATH         CSV data file              (default: "
                   "data_home_1_scheduled_GPS.csv)\n"
                << "  --no-save           Disable model checkpointing\n"
                << "  --help              Show this help message\n\n"
                << "Example:\n"
                << "  " << argv[0]
                << " --timesteps 500000 --w-economy 0.4 --output "
                   "models/eco_no_gps.pt\n";
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
    std::cout << "             OLA Trainer" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Creating simulation environment..." << std::endl;

    // ---- Build the training environment ----
    auto provider = SimulationBuilder()
                        .setClock(60.0)
                        .setDataSource(dataCsvPath)
                        .setRoom(20.0)
                        .addWall(5.0, 2.5, 0.3, 0.6)
                        .addWall(4.0, 2.5, 0.3, 0.6)
                        .addWall(5.0, 2.5, 0.3, 0.6)
                        .addWall(4.0, 2.5, 0.3, 0.6)
                        .addWindow(2.0, 1.8, 0.5)
                        .addHeater(2000.0)
                        .trainOLAModel(config)
                        .build();

    std::cout << "Simulation provider created successfully" << std::endl;
    std::cout << "Starting training simulation loop..." << std::endl;

    const auto room = provider.get<Room>();
    const auto clock = provider.get<IClock>();

    std::cout << "Running for " << config.totalTimesteps << " total timesteps.\n";

    for (int step = 0; step < config.totalTimesteps; ++step)
    {
        clock->simulate();
        room->simulate();

        // std::cout << "\r[TrainMainNoGPS] Room temp: " << room->getTemperature()
        //     << "C | Step: " << step + 1 << "/" << config.totalTimesteps
        //     << std::flush;
    }

    std::cout << "\n[TrainMainNoGPS] Training complete!" << std::endl;
    const auto consumptionService = provider.get<IConsumptionService>();
    const auto userComfortService = provider.get<IUserComfortService>();

    std::cout << "Final room temperature: " << room->getTemperature() << " °C" << std::endl;
    std::cout << "Average user comfort: " << userComfortService->getUserComfort() << "%" << std::endl;
    std::cout << "Total energy consumed: " << consumptionService->getTotalEnergyKWh() << " kWh" << std::endl;
    std::cout << "Total Cost energy: " << consumptionService->getTotalCost() * 0.000033 << " €" << std::endl;

    auto recorder = provider.get<IAIRecorder>();
    recorder->writeToCSV(outputDataPath);

    return 0;
}
