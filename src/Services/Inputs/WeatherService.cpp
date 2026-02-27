/**
 * @file WeatherService.cpp
 * @brief Implementation of the weather data service.
 */

#include "WeatherService.hpp"

using namespace POLA::Common;
using namespace POLA::Services::Inputs;
using namespace POLA::Interfaces;
using namespace POLA::Simulation;

WeatherData WeatherService::getInput()
{
    auto clock = _provider.get<IClock>();
    uint32_t currentTime = clock->getElapsedTimeSinceStart();
    auto dataManager = _provider.get<DataManager>();
    DataPoint dp = dataManager->getDataPointForTime(currentTime);
    return WeatherData{dp.outdoor_temp, dp.light_level};
}

int WeatherService::getIndexForTime(double time) const
{
    // Convert time from seconds to hours, assuming the CSV data is hourly
    double timeInHours = time / 3600.0;
    return static_cast<int>(timeInHours) % _weatherDataCache.size();
}

void WeatherService::loadWeatherDataFromCSV(const std::string &filePath)
{
    std::ifstream file(filePath);
    std::string line;
    std::vector<WeatherData> data;

    if (!file.is_open())
    {
        std::cerr << "Impossible to open the weather data file: " << filePath << std::endl;
        throw std::runtime_error("Failed to open weather data CSV file");
    }

    // Skip the header line
    std::getline(file, line);

    int count = 0;
    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string item;
        std::vector<std::string> row;

        // cut the line into items using ';' as delimiter
        while (std::getline(ss, item, ';'))
        {
            row.push_back(item);
        }

        // Mapping to the WeatherDataCSV struct based on the expected column indices
        if (row.size() >= 13)
        {
            double temperature;
            double sunlight;
            try {
                temperature = std::stod(row[3]);     // Index 3: Temperature (C)
                sunlight = std::stod(row[12]);       // Index 12: Enlightment (0.0 to 1.0)
            } catch (const std::exception& e) {
                std::cerr << "Error parsing line " << count << ": " << e.what() << " Skipping it." << std::endl;
                continue; // Skip this line and continue with the next one
            }
            WeatherDataCSV wd;
            wd.timestamp = row[0];
            wd.outTemperature = temperature;
            wd.sunlightIntensity = sunlight;
            _weatherDataCache.push_back(wd);
        }
    }

    std::cout << "Weather data loaded from CSV file: " << filePath << std::endl;
    std::cout << "Read " << _weatherDataCache.size() << " lines." << std::endl;
}
