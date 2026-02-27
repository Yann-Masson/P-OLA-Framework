/**
 * @file WeatherService.hpp
 * @brief Service providing outdoor weather and sunlight data.
 */

#pragma once

#include "AInputService.hpp"
#include "Common/DataTypes.hpp"
#include "Interfaces/IClock.hpp"
#include "Simulation/DataManager/DataManager.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

namespace POLA::Services::Inputs
{

    struct WeatherDataCSV
    {
        std::string timestamp;
        double outTemperature;
        double sunlightIntensity; // going from 0.0 to 1.0, representing the fraction of maximum sunlight intensity
    };

    class WeatherService : public AInputService<Common::WeatherData>
    {
    public:
        using AInputService<Common::WeatherData>::AInputService;
        Common::WeatherData getInput() override;
    };

} // namespace POLA::Services::Inputs
