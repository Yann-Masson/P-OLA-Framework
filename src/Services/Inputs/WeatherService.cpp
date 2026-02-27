/**
 * @file WeatherService.cpp
 * @brief Implementation of the weather data service.
 */

#include "WeatherService.hpp"

using namespace POLA::Common;
using namespace POLA::Services::Inputs;
using namespace POLA::Interfaces;

WeatherService::WeatherService(const forge::ProviderRef &provider)
    : AInputService<Common::WeatherData>(provider)
{
    const std::string weatherCsvPath = std::string(DATA_DIR) + "/weather_data.csv";
    loadWeatherDataFromCSV(weatherCsvPath);
}

WeatherData WeatherService::getInput()
{
    auto clock = _provider.get<IClock>();
    double currentTime = clock->getElapsedTimeSinceStart();

    int index = getIndexForTime(currentTime);
    if (index < 0 || index >= _weatherDataCache.size()) { // Should not happen due to modulo
        std::cerr << "Warning: Simulation time " << currentTime << " is out of bounds for weather data. Returning default values." << std::endl;
        return {
            .outTemperature = 20.0,
            .sunlightIntensity = 0.0
        };
    }
    return {
        .outTemperature = _weatherDataCache[index].outTemperature,
        .sunlightIntensity = _weatherDataCache[index].sunlightIntensity
    };
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
