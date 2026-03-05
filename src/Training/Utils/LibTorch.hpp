//
// Created by Yann on 05/03/2026.
//

#pragma once

#include <torch/torch.h>

namespace POLA::Training::Utils
{
    torch::Tensor normalLogProb(const torch::Tensor& x, const torch::Tensor& mean,
                                const torch::Tensor& std);

    torch::Tensor normalEntropy(const torch::Tensor& std);

    void orthogonalInit(torch::nn::Linear& layer, double gain = 1.0);
}
