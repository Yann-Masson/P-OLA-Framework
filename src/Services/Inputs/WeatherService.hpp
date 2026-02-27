/**
 * @file WeatherService.hpp
 * @brief Service providing outdoor weather and sunlight data.
 */

#pragma once

#include "AInputService.hpp"
#include "Common/DataTypes.hpp"
#include "Interfaces/IClock.hpp"
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
        explicit WeatherService(const forge::ProviderRef &provider);
        Common::WeatherData getInput() override;

    private:
        void loadWeatherDataFromCSV(const std::string &filePath);
        int getIndexForTime(double time) const;

        std::vector<WeatherDataCSV> _weatherDataCache; // Cache for weather data loaded from CSV
    };

} // namespace POLA::Services::Inputs
