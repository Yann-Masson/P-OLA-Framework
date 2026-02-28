/**
 * @file UserScheduleService.hpp
 * @brief Service providing user schedule data.
 */

#pragma once

#include "AInputService.hpp"
#include "Common/DataTypes.hpp"
#include "Simulation/DataManager/DataManager.hpp"
#include "Interfaces/IClock.hpp"

namespace POLA::Services::Inputs {

class UserScheduleService : public AInputService<Common::UserScheduleData>
{
public:
    using AInputService<Common::UserScheduleData>::AInputService;
    Common::UserScheduleData getInput() override;
};

} // namespace POLA::Services::Inputs
