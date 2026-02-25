/**
 * @file WeatherService.hpp
 * @brief Service providing outdoor weather and sunlight data.
 */

#pragma once

#include "AInputService.hpp"
#include "Common/DataTypes.hpp"

namespace POLA::Services::Inputs {

class WeatherService : public AInputService<Common::WeatherData>
{
public:
    using AInputService<Common::WeatherData>::AInputService;
    Common::WeatherData getInput() override;
};

} // namespace POLA::Services::Inputs
