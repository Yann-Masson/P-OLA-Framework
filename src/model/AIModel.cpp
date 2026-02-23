//
// Created by Yann on 16/02/2026.
//

#include "AIModel.hpp"

#include <filesystem>
#include <iostream>
#include <torch/torch.h>

AIModel::AIModel(std::string modelPath)
    : modelPath_(std::move(modelPath))
{
}

void AIModel::ensureLoaded()
{
    if (loaded_) {
        return;
    }

    namespace fs = std::filesystem;

    try {
        const fs::path modelPath(modelPath_);
        if (modelPath.has_parent_path()) {
            fs::create_directories(modelPath.parent_path());
        }

        if (!fs::exists(modelPath)) {
            std::cerr << "[AIModel] Model file not found at: " << modelPath.string() << std::endl;
            throw std::runtime_error("Model file not found");
        }

        module_ = torch::jit::load(modelPath.string());
        module_.eval();

        // Move to GPU if available
        if (torch::cuda::is_available()) {
            module_.to(torch::kCUDA);
            device_ = torch::kCUDA;
        } else if (torch::mps::is_available()) {
            module_.to(torch::kMPS);
            device_ = torch::kMPS;
        } else {
            device_ = torch::kCPU;
        }

        loaded_ = true;
    } catch (const c10::Error& e) {
        std::cerr << "[AIModel] Failed to load model: " << e.what() << std::endl;
        throw;
    }
}

double AIModel::predict(const State& state)
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
    }}, torch::TensorOptions().dtype(torch::kFloat32).device(device_));

    auto output = module_.forward({input}).toTensor();
    return output.squeeze().item<double>();
}
