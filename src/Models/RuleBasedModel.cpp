/**
 * @file RuleBasedModel.cpp
 * @brief Implementation of the rule-based prediction model.
 */

#include "RuleBasedModel.hpp"

using namespace POLA::Models;

double RuleBasedModel::predict(const Common::AIState& state)
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
