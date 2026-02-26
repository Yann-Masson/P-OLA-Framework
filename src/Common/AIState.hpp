/**
 * @file AIState.hpp
 * @brief Definition of the AIState struct representing the input state for the AI model.
 */

#pragma once

namespace POLA::Common {

    struct AIState {
        double tempIn;
        double tempOut;
        double electricityPrice;
        double gpsDistance;
        double userVelocity;
        double targetTemp;
    };

} // namespace POLA::Common
