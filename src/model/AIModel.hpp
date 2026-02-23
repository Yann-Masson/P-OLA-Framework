//
// Created by Yann on 16/02/2026.
//

#pragma once
#include "IAIModel.hpp"
#include <string>
#include <torch/script.h>

class AIModel : public IAIModel
{
public:
    explicit AIModel(std::string modelPath = "models/ai_model.pt");
    double predict(const State& state) override;

private:
    void ensureLoaded();

    torch::jit::script::Module module_;
    std::string modelPath_;
    bool loaded_ = false;
    torch::Device device_ = torch::kCPU;
};
