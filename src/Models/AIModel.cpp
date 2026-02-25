/**
 * @file AIModel.cpp
 * @brief Implementation of the TorchScript-based AI prediction model.
 */

#include <iostream>
#include <filesystem>
#include <torch/torch.h>

#include "AIModel.hpp"

namespace fs = std::filesystem;

namespace POLA::Models {

AIModel::AIModel(std::string modelPath)
    : _modelPath(std::move(modelPath))
{
}

void AIModel::ensureLoaded()
{
    if (_loaded) {
        return;
    }

    try {
        const fs::path modelPath(_modelPath);
        if (modelPath.has_parent_path()) {
            fs::create_directories(modelPath.parent_path());
        }

        if (!fs::exists(modelPath)) {
            std::cerr << "[AIModel] Model file not found at: " << modelPath.string() << std::endl;
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
    } catch (const c10::Error& e) {
        std::cerr << "[AIModel] Failed to load model: " << e.what() << std::endl;
        throw;
    }
}

double AIModel::predict(const Common::AIState& state)
{
    ensureLoaded();

    torch::NoGradGuard noGrad;
    auto input = torch::tensor({{
        static_cast<float>(state.tempIn),
        static_cast<float>(state.tempOut),
        static_cast<float>(state.electricityPrice),
        static_cast<float>(state.gpsDistance),
        static_cast<float>(state.userVelocity),
        static_cast<float>(state.targetTemp)
    }}, torch::TensorOptions().dtype(torch::kFloat32).device(_device));

    auto output = _module.forward({input}).toTensor();
    return output.squeeze().item<double>();
}

} // namespace POLA::Models
