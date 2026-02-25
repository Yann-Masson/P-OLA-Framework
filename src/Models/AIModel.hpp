/**
 * @file AIModel.hpp
 * @brief TorchScript-based AI model for predicting optimal thermostat behavior.
 */

#pragma once

#include <string>
#include <torch/script.h>

#include "Interfaces/IAIModel.hpp"

namespace POLA::Models {

class AIModel : public Interfaces::IAIModel
{
public:
    explicit AIModel(std::string modelPath = "models/ai_model.pt");
    double predict(const Common::AIState& state) override;

private:
    void ensureLoaded();

    bool _loaded = false;
    std::string _modelPath;
    torch::jit::script::Module _module;
    torch::Device _device = torch::kCPU;
};

} // namespace POLA::Models
