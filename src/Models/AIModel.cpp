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

void AIModel::ensureLoaded() {
  if (_loaded) {
    return;
  }

  try {
    const fs::path modelPath(_modelPath);
    if (modelPath.has_parent_path()) {
      fs::create_directories(modelPath.parent_path());
    }

    if (!fs::exists(modelPath)) {
      std::cerr << "[AIModel] Model file not found at: " << modelPath.string()
                << std::endl;
      throw std::runtime_error("Model file not found");
    }

    _module = torch::jit::load(modelPath.string());
    _module.eval();

    // Move to GPU if available
    if (torch::cuda::is_available()) {
      _module.to(torch::kCUDA);
      _device = torch::kCUDA;
    } else if (torch::mps::is_available()) {
      _module.to(torch::kMPS);
      _device = torch::kMPS;
    } else {
      _device = torch::kCPU;
    }

    _loaded = true;
  } catch (const c10::Error &e) {
    std::cerr << "[AIModel] Failed to load model: " << e.what() << std::endl;
    throw;
  }
}

double AIModel::predict(const Common::AIState &state) {
  ensureLoaded();

  torch::NoGradGuard noGrad;

  // Build raw 42-dim input matching the layout in normalizeState()
  // (normalization is handled inside the TorchScript forward() function).
  std::vector<float> rawFeatures;
  rawFeatures.reserve(42);

  rawFeatures.push_back(static_cast<float>(state.tempIn));
  rawFeatures.push_back(static_cast<float>(state.electricityPrice));
  rawFeatures.push_back(static_cast<float>(state.userDistanceKm));
  rawFeatures.push_back(static_cast<float>(state.userVelocityKmMin));

  for (const auto &wp : state.weather.forecast) {
    rawFeatures.push_back(static_cast<float>(wp.outdoorTemp));
    rawFeatures.push_back(static_cast<float>(wp.sunlightLuxIntensity));
  }

  rawFeatures.push_back(
      static_cast<float>(state.userPreferences.minTemperature));
  rawFeatures.push_back(
      static_cast<float>(state.userPreferences.maxTemperature));

  for (const bool present : state.userSchedule.userPresent) {
    rawFeatures.push_back(present ? 1.0f : 0.0f);
  }

  auto input =
      torch::tensor(
          rawFeatures,
          torch::TensorOptions().dtype(torch::kFloat32).device(_device))
          .unsqueeze(0); // [1, 18]

  auto output = _module.forward({input}).toTensor();
  return output.squeeze().item<double>();
}
