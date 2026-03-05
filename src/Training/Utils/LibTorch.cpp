/**
 * @file LibTorch.cpp
 * @brief Utility functions for PPO training using LibTorch.
 */

#include "LibTorch.hpp"

namespace POLA::Training::Utils
{
    torch::Tensor normalLogProb(const torch::Tensor& x, const torch::Tensor& mean,
                                const torch::Tensor& std)
    {
        const auto var = std * std;
        return -0.5 * ((x - mean).pow(2) / (var + 1e-8)) - std.log() -
            0.5 * std::log(2.0 * M_PI);
    }

    torch::Tensor normalEntropy(const torch::Tensor& std)
    {
        return 0.5 + 0.5 * std::log(2.0 * M_PI) + std.log();
    }

    void orthogonalInit(torch::nn::Linear& layer, double gain)
    {
        torch::nn::init::orthogonal_(layer->weight, gain);
        if (layer->bias.defined())
            torch::nn::init::zeros_(layer->bias);
    }
} // namespace POLA::Training
