/**
 * @file AIModel.hpp
 * @brief TorchScript-based AI model for predicting optimal thermostat behavior.
 */

#pragma once

#include <string>
#include <torch/script.h>
#include <forge/provider.hpp>

#include "Interfaces/IAIModel.hpp"

namespace POLA::Models
{

    class AIModel : public Interfaces::IAIModel
    {
    public:
        explicit AIModel(const forge::ProviderRef &provider, std::string modelPath = "models/ai_model.pt");
        double predict(const Common::AIState &state) override;

    private:
        void ensureLoaded();
        void detectModelType();

        bool _loaded = false;
        std::string _modelPath;
        torch::jit::script::Module _module;
        torch::Device _device = torch::kCPU;
        forge::ProviderRef _provider;

        enum class ModelType
        {
            OLA,
            POLA,
            Unknown
        };
        ModelType _modelType = ModelType::Unknown;
        int _expectedInputDim = 42; // Default to P-OLA
    };

} // namespace POLA::Models
