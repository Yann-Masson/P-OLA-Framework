/**
 * @file GPSService.hpp
 * @brief Service providing GPS location and velocity data.
 */

#pragma once

#include "AInputService.hpp"
#include "Common/DataTypes.hpp"

namespace POLA::Services::Inputs {

class GPSService : public AInputService<Common::GPSData>
{
public:
    using AInputService<Common::GPSData>::AInputService;
    Common::GPSData getInput() override;
};

} // namespace POLA::Services::Inputs

