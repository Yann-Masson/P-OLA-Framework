/**
 * @file ProviderSetup.cpp
 * @brief Implementation of the composable simulation builder.
 */

#include "ProviderSetup.hpp"

#include <filesystem>
#include <iostream>

#include "Services/Clock.hpp"
#include "Services/ConsumptionService.hpp"
#include "Services/Inputs/EnergyPriceService.hpp"
#include "Services/Inputs/GPSService.hpp"
#include "Services/Inputs/UserPreferenceService.hpp"
#include "Services/Inputs/UserScheduleService.hpp"
#include "Services/Inputs/WeatherService.hpp"
#include "Services/UserComfortService.hpp"


#include "Simulation/DataManager/DataManager.hpp"
#include "Simulation/Room/Room.hpp"
#include "Simulation/SmartThermostat/AIRecorder.hpp"
#include "Simulation/SmartThermostat/SmartThermostat.hpp"
#include "Simulation/TemperatureFactor/Heater.hpp"
#include "Simulation/TemperatureFactor/Wall.hpp"
#include "Simulation/TemperatureFactor/Window.hpp"


#include "Models/AIModel.hpp"
#include "Models/RuleBasedModel.hpp"

#include "Interfaces/IAIModel.hpp"
#include "Interfaces/IAIRecorder.hpp"
#include "Interfaces/IClock.hpp"

#include "Training/P-OLA/POLATrainingAgent.hpp"
#include "Training/OLA/OLATrainingAgent.hpp"

using namespace POLA::Common;
using namespace POLA::Interfaces;
using namespace POLA::Services;
using namespace POLA::Services::Inputs;
using namespace POLA::Simulation;
using namespace POLA::Simulation::TemperatureFactor;

using namespace POLA::Common;

// ============================================================================
// SimulationBuilder
// ============================================================================

SimulationBuilder &SimulationBuilder::setClock(const double fixedDtSeconds) {
  _clockDt = fixedDtSeconds;
  return *this;
}

SimulationBuilder &
SimulationBuilder::setDataSource(const std::string &csvPath) {
  _dataCsvPath = csvPath;
  return *this;
}

SimulationBuilder &
SimulationBuilder::setRoom(const double startingTemperature) {
  _startingRoomTemp = startingTemperature;
  _hasRoom = true;
  return *this;
}

SimulationBuilder &SimulationBuilder::addWall(const double width,
                                              const double height,
                                              const double uValue,
                                              const double solarAbsorptance) {
  _registrations.push_back(
      [width, height, uValue, solarAbsorptance](ProviderBuilder &builder) {
        builder.addService<ITemperatureFactor, Wall>(
            std::function<std::shared_ptr<Wall>(ProviderRef)>(
                [width, height, uValue, solarAbsorptance](ProviderRef p) {
                  return std::make_shared<Wall>(p, width, height, uValue,
                                                solarAbsorptance);
                }));
      });
  return *this;
}

SimulationBuilder &SimulationBuilder::addWindow(const double area,
                                                const double uValue,
                                                const double shgc) {
  _registrations.push_back([area, uValue, shgc](ProviderBuilder &builder) {
    builder.addService<ITemperatureFactor, Window>(
        std::function<std::shared_ptr<Window>(ProviderRef)>(
            [area, uValue, shgc](ProviderRef p) {
              return std::make_shared<Window>(p, area, uValue, shgc);
            }));
  });
  return *this;
}

SimulationBuilder &SimulationBuilder::addHeater(const double maxPowerW) {
  _registrations.push_back([maxPowerW](ProviderBuilder &builder) {
    builder.addService<ITemperatureFactor, Heater>(
        std::function<std::shared_ptr<Heater>(ProviderRef)>(
            [maxPowerW](ProviderRef p) {
              return std::make_shared<Heater>(p, maxPowerW);
            }));
  });
  return *this;
}

SimulationBuilder &SimulationBuilder::useAIModel(const std::string &modelPath) {
  _modelPath = modelPath;
  _useRuleBased = false;
  return *this;
}

SimulationBuilder &SimulationBuilder::useRuleBasedModel() {
  _modelPath.clear();
  _useRuleBased = true;
  return *this;
}

SimulationBuilder &
SimulationBuilder::trainPOLAModel(const Training::TrainingConfig &config) {
  _modelPath.clear();
  _useRuleBased = false;
  _trainingPOLAAgent = true;
  _trainingOLAAgent = false;

  // Register the POLATrainingAgent as IAIModel
  _registrations.push_back([config](ProviderBuilder &pb) {
    pb.addService<IAIModel, Training::POLA::POLATrainingAgent>(
        std::function<std::shared_ptr<Training::POLA::POLATrainingAgent>(ProviderRef)>(
            [config](ProviderRef p) {
              return std::make_shared<Training::POLA::POLATrainingAgent>(p, config);
            }));
  });

  return *this;
}

SimulationBuilder &SimulationBuilder::trainOLAModel(
    const Training::TrainingConfig &config) {
  _modelPath.clear();
  _useRuleBased = false;
  _trainingPOLAAgent = false;
  _trainingOLAAgent = true;

  // Register OLATrainingAgent as IAIModel
  _registrations.push_back([config](ProviderBuilder &pb) {
    pb.addService<IAIModel, Training::OLA::OLATrainingAgent>(
        std::function<std::shared_ptr<Training::OLA::OLATrainingAgent>(
            ProviderRef)>([config](ProviderRef p) {
          return std::make_shared<Training::OLA::OLATrainingAgent>(p, config);
        }));
  });

  return *this;
}

Provider SimulationBuilder::build() {
  namespace fs = std::filesystem;

  std::cout << "[SimulationBuilder] Building provider with configuration:\n"
            << "  Clock dt: "
            << (_clockDt > 0.0 ? std::to_string(_clockDt) + "s" : "N/A") << "\n"
            << "  Data CSV path:    "
            << (_dataCsvPath.empty() ? "Default dataset" : _dataCsvPath) << "\n"
            << "  Model path:       "
            << (_modelPath.empty() ? (_useRuleBased ? "Using rule-based model"
                                                    : "No model specified")
                                   : _modelPath)
            << "\n"
            << "  Starting room temp: "
            << (_hasRoom ? std::to_string(_startingRoomTemp) + "C" : "N/A")
            << "\n"
            << "  Has training agent: " << (_trainingPOLAAgent ? "Yes" : "No")
            << "\n"
            << "  Registered temperature factors: " << _registrations.size()
            << "\n"
            << std::endl;

  // --- Clock ---
  std::shared_ptr<IClock> clockService = std::make_shared<Clock>(_clockDt);

  // --- Data source ---
  std::string dataPath = _dataCsvPath;
  if (dataPath.empty()) {
    dataPath = std::string(DATA_DIR) + "/data_home_1_scheduled.csv";
  }
  auto dataManager = std::make_shared<DataManager>(dataPath);

  std::cout << "[SimulationBuilder] DataManager initialized with data from: "
            << dataPath << std::endl;

  std::cout << "[SimulationBuilder] Building provider with "
            << _registrations.size() << " temperature factors and "
            << (_trainingPOLAAgent
                    ? "a training agent"
                    : (_useRuleBased ? "a rule-based model" : "an AI model"))
            << std::endl;
  // --- Core services ---
  auto builder =
      ProviderBuilder()
          .addService(clockService)
          .addService<IInputService<EnergyPriceData>, EnergyPriceService>()
          .addService<IInputService<WeatherData>, WeatherService>()
          .addService<IInputService<GPSData>, GPSService>()
          .addService<IInputService<UserPreferenceData>,
                      UserPreferenceService>()
          .addService<IInputService<UserScheduleData>, UserScheduleService>()
          .addService<IConsumptionService, ConsumptionService>()
          .addService<IUserComfortService, UserComfortService>();

  std::cout << "[SimulationBuilder] Core services registered: Clock, "
               "EnergyPriceService, WeatherService, GPSService, "
               "UserPreferenceService, UserScheduleService, "
               "ConsumptionService, UserComfortService"
            << std::endl;

  // --- Temperature factors (Lego bricks) ---
  for (auto &registration : _registrations) {
    registration(builder);
  }

  std::cout << "[SimulationBuilder] Registered " << _registrations.size()
            << " temperature factors (walls, windows, heaters)" << std::endl;

  // --- AI model (only when NOT using the training agent) ---
  if (!_trainingPOLAAgent) {
    if (!_modelPath.empty() && fs::exists(_modelPath)) {
      std::cout << "[SimulationBuilder] Using AI model from: " << _modelPath
                << std::endl;
      auto modelPath = _modelPath;
      builder.addService<IAIModel, Models::AIModel>(
          std::function<std::shared_ptr<Models::AIModel>(ProviderRef)>(
              [modelPath](ProviderRef p) {
                return std::make_shared<Models::AIModel>(p, modelPath);
              }));
    } else {
      std::cout << "[SimulationBuilder] Using rule-based model" << std::endl;
      builder.addService<IAIModel, Models::RuleBasedModel>();
    }
  }

  std::cout << "[SimulationBuilder] AI model registered: "
            << (_trainingPOLAAgent
                    ? "N/A (training agent handles this)"
                    : (_useRuleBased ? "RuleBasedModel"
                                     : "AIModel from " + _modelPath))
            << std::endl;

  // --- AI Recorder ---
  builder.addService<POLA::Interfaces::IAIRecorder, Simulation::AIRecorder>();

  std::cout << "[SimulationBuilder] AIRecorder service registered" << std::endl;

  builder.addService<ISmartThermostat, SmartThermostat>();
  std::cout << "[SimulationBuilder] SmartThermostat service registered"
            << std::endl;

  // --- Room ---
  if (_hasRoom) {
    auto startTemp = _startingRoomTemp;
    builder.addService<Room>(std::function<std::shared_ptr<Room>(ProviderRef)>(
        [startTemp](ProviderRef p) {
          return std::make_shared<Room>(p, startTemp);
        }));
  }

  std::cout << "[SimulationBuilder] Room service registered with starting "
               "temperature: "
            << (_hasRoom ? std::to_string(_startingRoomTemp) + "C" : "N/A")
            << std::endl;

  // --- DataManager ---
  builder.addService<DataManager, DataManager>(dataManager);

  std::cout
      << "[SimulationBuilder] DataManager service registered with data from: "
      << dataPath << std::endl;

  return builder.build();
}

// ============================================================================
// Convenience wrapper (backward compatibility)
// ============================================================================

Provider createSimulationProvider(const double timeScale,
                                  const std::string &dataCsvPath,
                                  const std::string &modelPath,
                                  const double startingRoomTemp) {
  auto builder =
      SimulationBuilder()
          .setClock(timeScale)
          .setDataSource(dataCsvPath)
          .setRoom(startingRoomTemp)
          // Standard rectangular room: 4 walls (5m×2.5m and 4m×2.5m pairs)
          .addWall(5.0, 2.5, 0.3, 0.6)
          .addWall(4.0, 2.5, 0.3, 0.6)
          .addWall(5.0, 2.5, 0.3, 0.6)
          .addWall(4.0, 2.5, 0.3, 0.6)
          // 1 window
          .addWindow(2.0, 1.8, 0.5)
          // 1 heater (2000W)
          .addHeater(2000.0);

  // AI or rule-based model
  if (!modelPath.empty()) {
    builder.useAIModel(modelPath);
  } else {
    builder.useRuleBasedModel();
  }

  return builder.build();
}
