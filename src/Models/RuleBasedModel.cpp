//
// Created by Yann on 16/02/2026.
//

#include "RuleBasedModel.hpp"

double RuleBasedModel::predict(const State& state)
{
    // Simple rule-based logic for demonstration
    if (state.electricityPrice > 0.20) {
        return 0.0; // Do not consume energy
    } else if (state.electricityPrice < 0.10) {
        return 1.0; // Consume maximum energy
    } else {
        return 0.5; // Moderate energy consumption
    }
}
