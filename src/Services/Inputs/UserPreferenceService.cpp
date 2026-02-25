/**
 * @file UserPreferenceService.cpp
 * @brief Implementation of the user preference data service.
 */

#include "UserPreferenceService.hpp"

using namespace POLA::Common;
using namespace POLA::Services::Inputs;

UserPreferenceData UserPreferenceService::getInput()
{
    return {
        .minTemperature = 22.0,
        .maxTemperature = 28.0
    };
}
