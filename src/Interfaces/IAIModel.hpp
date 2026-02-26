/**
 * @file IAIModel.hpp
 * @brief Interface for AI prediction models used by the smart thermostat.
 */

#pragma once

#include "Common/AIState.hpp"

namespace POLA::Interfaces {

class IAIModel {
public:
    virtual ~IAIModel() = default;
    virtual double predict(const Common::AIState& state) = 0;
};

} // namespace POLA::Interfaces
