/**
 * @file ProviderSetup.hpp
 * @brief Composable simulation builder for creating service providers.
 *
 * SimulationBuilder provides a fluent, "Lego-like" API for assembling
 * a simulation environment. Each component (wall, window, heater, etc.)
 * is an independent brick that can be added or omitted as needed.
 *
 * Both the real simulator and the training environment use the same builder,
 * ensuring consistent physics regardless of the mode.
 */

#pragma once

#include <string>
#include <vector>
#include <functional>

#include <forge/provider.hpp>
#include <forge/provider_builder.hpp>

#include "Training/TrainingConfig.hpp"

namespace POLA::Common
{
    /**
     * @brief Composable builder for assembling simulation providers.
     *
     * Usage example:
     * @code
     * auto provider = SimulationBuilder()
     *     .setClock(900.0)
     *     .setDataSource("data/file.csv")
     *     .setRoom(20.0)
     *     .addWall(5.0, 2.5, 0.3, 0.6)
     *     .addWall(4.0, 2.5, 0.3, 0.6)
     *     .addWall(5.0, 2.5, 0.3, 0.6)
     *     .addWall(4.0, 2.5, 0.3, 0.6)
     *     .addWindow(2.0, 1.8, 0.5)
     *     .addHeater(2000.0)
     *     .useAIModel("models/ai_model.pt")
     *     .build();
     * @endcode
     */
    class SimulationBuilder
    {
    public:
        /// Set the simulation clock timescale (e.g., 900 = 1 real second = 15
        /// minutes).
        SimulationBuilder& setClock(double fixedDtSeconds = 60.0);

        /// Set the data source CSV file path for input services.
        SimulationBuilder& setDataSource(const std::string& csvPath);

        /// Set the room starting temperature.
        SimulationBuilder& setRoom(double startingTemperature = 20.0);

        /// Add a wall temperature factor (each call adds one wall Lego brick).
        SimulationBuilder& addWall(double width, double height, double uValue,
                                   double solarAbsorptance);

        /// Add a window temperature factor (each call adds one window Lego brick).
        SimulationBuilder& addWindow(double area, double uValue, double shgc);

        /// Add a heater temperature factor (each call adds one heater Lego brick).
        SimulationBuilder& addHeater(double maxPowerW = 2000.0);

        /// Use a trained AI model for the smart thermostat (loads TorchScript .pt
        /// file).
        SimulationBuilder& useAIModel(const std::string& modelPath);

        /// Use a rule-based model instead of AI for the smart thermostat.
        SimulationBuilder& useRuleBasedModel();

        /// Use the Reinforcement Learning PPO Agent for training!
        SimulationBuilder& trainPOLAModel(const Training::TrainingConfig& config);

        /// Use the GPS-free PPO Agent for ablation training (no gpsDistance /
        /// userVelocity).
        SimulationBuilder&
        trainOLAModel(const Training::TrainingConfig& config);

        /// Build the provider with all registered components.
        forge::Provider build();

    private:
        double _clockDt = 60.0;
        std::string _dataCsvPath;
        double _startingRoomTemp = 20.0;
        bool _hasRoom = false;
        std::string _modelPath;
        bool _useRuleBased = false;

        /// True if the user called .trainPOLAModel() — means the IAIModel was
        /// already pushed into _registrations.
        bool _trainingPOLAAgent = false;

        /// True if the user called .trainOLAModel().
        bool _trainingOLAAgent = false;

        // Deferred registrations stored as lambdas applied to the ProviderBuilder
        std::vector<std::function<void(forge::ProviderBuilder&)>> _registrations;
    };

    // ---- Convenience wrapper for backward compatibility ----

    /**
     * @brief Creates a fully configured service provider with the default room
     * layout.
     *
     * @param timeScale Simulation timescale (e.g., 900 means 1 real second = 15
     * minutes)
     * @param dataCsvPath Path to the CSV data file (optional)
     * @param modelPath Path to the trained AI model (optional, if empty uses
     * rule-based model)
     * @param startingRoomTemp Initial room temperature in Celsius (default: 20.0)
     * @return forge::Provider Configured service provider ready for simulation
     */
    forge::Provider createSimulationProvider(double timeScale = 900.0,
                                             const std::string& dataCsvPath = "",
                                             const std::string& modelPath = "",
                                             double startingRoomTemp = 20.0);
} // namespace POLA::Common
