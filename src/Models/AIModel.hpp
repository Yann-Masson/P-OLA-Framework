//
// Created by Yann on 16/02/2026.
//

#pragma once
#include "../Interfaces/IAIModel.hpp"
#include <string>
#include <torch/script.h>

class AIModel : public IAIModel
{
public:
    explicit AIModel(std::string modelPath = "models/ai_model.pt");
    double predict(const State& state) override;

private:
    void ensureLoaded();

    torch::jit::script::Module _module;
    std::string _modelPath;
    bool _loaded = false;
    torch::Device _device = torch::kCPU;
};
