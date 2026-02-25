/**
 * @file UserPreferenceService.cpp
 * @brief Implementation of the user preference data service.
 */

#include "UserPreferenceService.hpp"

namespace POLA::Services::Inputs {

Common::UserPreferenceData UserPreferenceService::getInput()
{
    return {
        .minTemperature = 22.0,
        .maxTemperature = 28.0
    };
}

} // namespace POLA::Services::Inputs
