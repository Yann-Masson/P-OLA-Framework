/**
 * @file RuleBasedModel.hpp
 * @brief Simple rule-based model as a fallback alternative to AI prediction.
 */

#pragma once

#include "Interfaces/IAIModel.hpp"

namespace POLA::Models {

class RuleBasedModel : public Interfaces::IAIModel
{
public:
    double predict(const Common::AIState& state) override;
};

} // namespace POLA::Models
