/**
 * @file RuleBasedModel.cpp
 * @brief Implementation of the rule-based prediction model.
 */

#include "RuleBasedModel.hpp"

#include <algorithm>

using namespace POLA::Models;

double RuleBasedModel::predict(const Common::AIState& state)
{
    // --- 1. Compute temperature error ---
    // Positive error means it's too cold (need more heat)
    const auto targetTemp = 21.0;
    double tempError = targetTemp - state.tempIn;

    // --- 2. Base power from temperature error ---
    // Map error linearly: 0 error → 0.5, ±10°C → 0.0 / 1.0
    double basePower = 0.5 + (tempError / 20.0);
    basePower = std::clamp(basePower, 0.0, 1.0);

    // --- 3. Outdoor temperature feed-forward ---
    // Colder outside → more heat loss → boost power slightly
    double outdoorBoost = 0.0;
    if (state.tempOut < 0.0)
    {
        outdoorBoost = std::clamp(-state.tempOut / 40.0, 0.0, 0.2); // up to +0.2
    }

    // --- 4. Electricity price penalty ---
    // High price → reduce power to save cost (soft reduction)
    // Assumes price range roughly 0–100 (cents/kWh or similar unit)
    double pricePenalty = 0.0;
    if (state.electricityPrice > 50.0)
    {
        pricePenalty = std::clamp((state.electricityPrice - 50.0) / 100.0, 0.0, 0.3);
    }

    // --- 5. GPS / distance factor ---
    // Far from destination → no need to pre-heat aggressively
    // Close to destination → comfort matters more
    double distanceFactor = 0.0;
    if (state.gpsDistance > 10.0)
    {
        distanceFactor = -std::clamp((state.gpsDistance - 10.0) / 50.0, 0.0, 0.2);
    }

    // --- 6. Velocity factor ---
    // At high speed, cabin heat loss increases (wind chill effect on body)
    // Boost heating slightly at highway speeds
    double velocityBoost = 0.0;
    if (state.userVelocity > 80.0)
    {
        velocityBoost = std::clamp((state.userVelocity - 80.0) / 120.0, 0.0, 0.1);
    }

    // --- 7. Combine all factors ---
    const double power = basePower + outdoorBoost - pricePenalty + distanceFactor + velocityBoost;

    return std::clamp(power, 0.0, 1.0);
}
