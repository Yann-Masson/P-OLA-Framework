/**
 * @file AIModel.cpp
 * @brief Implementation of the TorchScript-based AI prediction model.
 */

#include <filesystem>
#include <iostream>
#include <torch/torch.h>
#include <vector>

#include "AIModel.hpp"

namespace fs = std::filesystem;

using namespace POLA::Models;

AIModel::AIModel(const forge::ProviderRef &provider, std::string modelPath)
    : _modelPath(std::move(modelPath)), _provider(provider) {}

void AIModel::ensureLoaded()
{
    if (_loaded)
    {
        return;
    }

    try
    {
        const fs::path modelPath(_modelPath);
        if (modelPath.has_parent_path())
        {
            fs::create_directories(modelPath.parent_path());
        }

        if (!fs::exists(modelPath))
        {
            std::cerr << "[AIModel] Model file not found at: " << modelPath.string()
                      << std::endl;
            throw std::runtime_error("Model file not found");
        }

        _module = torch::jit::load(modelPath.string());
        _module.eval();

        // Move to GPU if available
        if (torch::cuda::is_available())
        {
            _module.to(torch::kCUDA);
            _device = torch::kCUDA;
        }
        else if (torch::mps::is_available())
        {
            _module.to(torch::kMPS);
            _device = torch::kMPS;
        }
        else
        {
            _device = torch::kCPU;
        }

        // Detect model type (OLA vs P-OLA) based on expected input dimensions
        detectModelType();

        _loaded = true;
    }
    catch (const c10::Error &e)
    {
        std::cerr << "[AIModel] Failed to load model: " << e.what() << std::endl;
        throw;
    }
}

void AIModel::detectModelType()
{
    // Try to infer model type by testing with dummy inputs of different sizes
    try
    {
        torch::NoGradGuard noGrad;

        // Try 24-dim input (OLA model)
        try
        {
            auto testInput24 = torch::zeros({1, 24},
                                            torch::TensorOptions().dtype(torch::kFloat32).device(_device));
            auto testOutput = _module.forward({testInput24}).toTensor();
            _modelType = ModelType::OLA;
            _expectedInputDim = 24;
            std::cout << "[AIModel] Detected OLA model (24 dimensions)" << std::endl;
            return;
        }
        catch (...)
        {
            // Not an OLA model, continue
        }

        // Try 42-dim input (P-OLA model)
        try
        {
            auto testInput42 = torch::zeros({1, 42},
                                            torch::TensorOptions().dtype(torch::kFloat32).device(_device));
            auto testOutput = _module.forward({testInput42}).toTensor();
            _modelType = ModelType::POLA;
            _expectedInputDim = 42;
            std::cout << "[AIModel] Detected P-OLA model (42 dimensions)" << std::endl;
            return;
        }
        catch (...)
        {
            // Not a P-OLA model either
        }

        std::cerr << "[AIModel] Warning: Could not detect model type, defaulting to P-OLA (42 dims)" << std::endl;
        _modelType = ModelType::POLA;
        _expectedInputDim = 42;
    }
    catch (const std::exception &e)
    {
        std::cerr << "[AIModel] Error during model type detection: " << e.what() << std::endl;
        _modelType = ModelType::POLA;
        _expectedInputDim = 42;
    }
}

double AIModel::predict(const Common::AIState &state)
{
    ensureLoaded();

    torch::NoGradGuard noGrad;

    std::vector<float> rawFeatures;

    if (_modelType == ModelType::OLA)
    {
        // OLA model: Only 24 dimensions (user schedule only)
        rawFeatures.reserve(24);

        for (const bool present : state.userSchedule.userPresent)
        {
            rawFeatures.push_back(present ? 1.0f : 0.0f);
        }
        
        // Validate OLA input
        if (rawFeatures.size() != 24) {
            std::cerr << "[AIModel] ERROR: OLA model expects 24 features, got " 
                      << rawFeatures.size() << std::endl;
        }
    }
    else
    {
        // P-OLA model: Full 42 dimensions
        // (normalization is handled inside the TorchScript forward() function)
        rawFeatures.reserve(42);

        rawFeatures.push_back(static_cast<float>(state.tempIn));
        rawFeatures.push_back(static_cast<float>(state.electricityPrice));
        rawFeatures.push_back(static_cast<float>(state.userDistanceKm));
        rawFeatures.push_back(static_cast<float>(state.userVelocityKmMin));

        for (const auto &wp : state.weather.forecast)
        {
            rawFeatures.push_back(static_cast<float>(wp.outdoorTemp));
            rawFeatures.push_back(static_cast<float>(wp.sunlightLuxIntensity));
        }

        rawFeatures.push_back(
            static_cast<float>(state.userPreferences.minTemperature));
        rawFeatures.push_back(
            static_cast<float>(state.userPreferences.maxTemperature));

        for (const bool present : state.userSchedule.userPresent)
        {
            rawFeatures.push_back(present ? 1.0f : 0.0f);
        }
        
        // Validate P-OLA input
        if (rawFeatures.size() != 42) {
            std::cerr << "[AIModel] ERROR: P-OLA model expects 42 features, got " 
                      << rawFeatures.size() 
                      << " (weather forecast: " << state.weather.forecast.size() 
                      << ", schedule: " << state.userSchedule.userPresent.size() << ")"
                      << std::endl;
        }
    }

    auto input =
        torch::tensor(
            rawFeatures,
            torch::TensorOptions().dtype(torch::kFloat32).device(_device))
            .unsqueeze(0);

    auto output = _module.forward({input}).toTensor();
    double prediction = output.squeeze().item<double>();
    
    // Debug: Log predictions periodically
    static int callCount = 0;
    callCount++;
    if (callCount % 500 == 0) {
        std::cout << "\n[AIModel Debug] Prediction #" << callCount << ":\n";
        std::cout << "  Model type: " << (_modelType == ModelType::OLA ? "OLA" : "P-OLA") << "\n";
        std::cout << "  Raw prediction: " << prediction << "\n";
        if (_modelType == ModelType::POLA) {
            std::cout << "  TempIn: " << state.tempIn << " °C\n";
            std::cout << "  Price: " << state.electricityPrice << "\n";
            std::cout << "  User distance: " << state.userDistanceKm << " km\n";
        }
        std::cout << "  Schedule (first 6h): ";
        for (int i = 0; i < std::min(6, (int)state.userSchedule.userPresent.size()); i++) {
            std::cout << (state.userSchedule.userPresent[i] ? "1" : "0") << " ";
        }
        std::cout << "\n" << std::flush;
    }
    
    return prediction;
}
