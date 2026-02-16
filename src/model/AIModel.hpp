//
// Created by Yann on 16/02/2026.
//

#pragma once
#include "IAIModel.hpp"

class AIModel : public IAIModel
{
public:
    double predict(const State& state) override;
};
