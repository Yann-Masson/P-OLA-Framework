/**
 * @file UserPreferenceService.hpp
 * @brief Service providing user temperature preference data.
 */

#pragma once

#include "AInputService.hpp"
#include "Common/DataTypes.hpp"

namespace POLA::Services::Inputs {

class UserPreferenceService : public AInputService<Common::UserPreferenceData>
{
public:
    using AInputService<Common::UserPreferenceData>::AInputService;
    Common::UserPreferenceData getInput() override;
};

} // namespace POLA::Services::Inputs
