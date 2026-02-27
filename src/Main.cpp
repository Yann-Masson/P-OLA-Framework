/**
 * @file Main.cpp
 * @brief Entry point for the P-OLA smart thermostat simulator.
 */

#include <chrono>
#include <iostream>
#include <filesystem>
#include <thread>

#include <torch/torch.h>
#include <torch/script.h>
#include <forge/provider_builder.hpp>

#include "Models/AIModel.hpp"
#include "Common/DataTypes.hpp"

#include "Services/Clock.hpp"
#include "Services/ConsumptionService.hpp"

#include "Services/Inputs/GPSService.hpp"
#include "Services/Inputs/WeatherService.hpp"
#include "Services/Inputs/EnergyPriceService.hpp"
#include "Services/Inputs/UserPreferenceService.hpp"

#include "Simulation/Room/Room.hpp"
#include "Simulation/TemperatureFactor/Wall.hpp"
#include "Simulation/TemperatureFactor/Heater.hpp"
#include "Simulation/TemperatureFactor/Window.hpp"
#include "Simulation/SmartThermostat/SmartThermostat.hpp"

using namespace POLA::Common;
using namespace POLA::Interfaces;
using namespace POLA::Models;
using namespace POLA::Services;
using namespace POLA::Services::Inputs;
using namespace POLA::Simulation;
using namespace POLA::Simulation::TemperatureFactor;

// Define a simple model struct for TorchScript
struct TinyModelImpl : torch::nn::Module
{
    TinyModelImpl()
    {
        fc1 = register_module("fc1", torch::nn::Linear(6, 16));
        fc2 = register_module("fc2", torch::nn::Linear(16, 1));
    }

    torch::Tensor forward(torch::Tensor x)
    {
        x = torch::relu(fc1->forward(x));
        x = fc2->forward(x);
        return x;
    }

    torch::nn::Linear fc1{nullptr}, fc2{nullptr};
};

TORCH_MODULE(TinyModel);

static std::string ensureTinyModel(const std::string &modelPath)
{
    namespace fs = std::filesystem;

    const fs::path path(modelPath);
    if (path.has_parent_path())
    {
        fs::create_directories(path.parent_path());
    }

    if (fs::exists(path))
    {
        return modelPath;
    }

    std::cout << "[INFO] Creating tiny model..." << std::endl;

    TinyModel model;
    model->eval();

    // Save using TorchScript serialization via torch::jit::Module
    // We build a jit::Module manually from the nn::Module parameters
    torch::jit::script::Module jit_module("TinyModel");

    // Register submodules to match the nn::Module structure
    auto fc1_jit = torch::jit::script::Module("fc1");
    fc1_jit.register_parameter("weight", model->fc1->weight.clone(), false);
    fc1_jit.register_parameter("bias", model->fc1->bias.clone(), false);

    auto fc2_jit = torch::jit::script::Module("fc2");
    fc2_jit.register_parameter("weight", model->fc2->weight.clone(), false);
    fc2_jit.register_parameter("bias", model->fc2->bias.clone(), false);

    jit_module.register_module("fc1", fc1_jit);
    jit_module.register_module("fc2", fc2_jit);

    // Define forward in TorchScript
    jit_module.define(R"(
        def forward(self, x):
            x = torch.matmul(x, self.fc1.weight.t()) + self.fc1.bias
            x = torch.relu(x)
            x = torch.matmul(x, self.fc2.weight.t()) + self.fc2.bias
            return x
    )");

    jit_module.save(path.string());
    std::cout << "[SUCCESS] Tiny model saved to: " << path.string() << std::endl;

    return modelPath;
}

void weatherServiceTest(const forge::Provider &provider)
{
    std::cout << "\n--- Weather Service Test ---" << std::endl;
    auto weatherService = provider.get<IInputService<WeatherData>>();
    auto clock = provider.get<IClock>();

    std::cout << "Testing weather service with simulated time progression:" << std::endl;
    for (int i = 0; i < 5; ++i)
    {
        WeatherData weather = weatherService->getInput();
        std::cout << "[Time: " << clock->getElapsedTimeSinceStart() << "s] "
                  << "Temperature: " << weather.outTemperature << "°C, "
                  << "Sunlight: " << weather.sunlightIntensity << " W/m²" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        clock->simulate();
    }
}

int main()
{
    const auto modelPath = ensureTinyModel("models/ai_model.pt");
    AIModel model(modelPath);

    constexpr AIState state {
        21.0,
        10.0,
        0.25,
        2.0,
        1.2,
        22.0
    };

    std::cout << "[AIModel] Prediction: " << model.predict(state) << std::endl;

    auto simulationClockService = std::make_shared<Clock>(86400); // 1 real second = 1 simulated day

    const auto provider = forge::ProviderBuilder()
                              .addService<IClock>(simulationClockService)
                              .addService<IInputService<EnergyPriceData>, EnergyPriceService>()
                              .addService<IInputService<WeatherData>, WeatherService>()
                              .addService<IInputService<GPSData>, GPSService>()
                              .addService<IInputService<UserPreferenceData>, UserPreferenceService>()
                              .addService<IConsumptionService, ConsumptionService>()
                              .addMultiService<ITemperatureFactor, Heater>()
                              .addMultiService<ITemperatureFactor, Wall>()
                              .addMultiService<ITemperatureFactor, Wall>()
                              .addMultiService<ITemperatureFactor, Wall>()
                              .addMultiService<ITemperatureFactor, Wall>()
                              .addMultiService<ITemperatureFactor, Window>()
                              .addService<IAIModel, AIModel>()
                              .addService<ISmartThermostat, SmartThermostat>()
                              .addService<Room>()
                              .build();

    std::cout << "Service Provider initialized with services:" << std::endl;
    std::cout << "Temperature factors registered:" << std::endl;
    for (const auto &factor : provider.getAll<ITemperatureFactor>())
    {
        std::cout << " - " << typeid(*factor).name() << std::endl;
    }

    std::cout << "Simulation clock initialized at time: " << provider.get<IClock>()->getElapsedTime() << " seconds" << std::endl;
    std::cout << "Energy price service initialized with current price: $" << provider.get<IInputService<EnergyPriceData>>()->getInput().pricePerKWh << " per kWh" << std::endl;
    std::cout << "Weather service initialized with current temperature: " << provider.get<IInputService<WeatherData>>()->getInput().outTemperature << "°C" << std::endl;
    std::cout << "GPS service initialized with current location: (" << provider.get<IInputService<GPSData>>()->getInput().distanceKm << " km)" << std::endl;
    std::cout << "User preference service initialized with preferred temperature: " << provider.get<IInputService<UserPreferenceData>>()->getInput().maxTemperature << "°C" << std::endl;
    std::cout << "Consumption service initialized with total energy: " << provider.get<IConsumptionService>()->getTotalEnergyKWh() << " kWh and total cost: $" << provider.get<IConsumptionService>()->getTotalCost() << std::endl;

    auto room = provider.get<Room>();

    weatherServiceTest(provider); // Test the weather service with simulated time progression

    return 0;
}
