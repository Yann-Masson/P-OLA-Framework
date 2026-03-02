/**
 * @file DataManager.hpp
 * @brief Data manager for handling simulation data.
 */

#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

namespace POLA::Simulation
{

    struct DataPoint
    {
        std::string timestamp; // for indexing, not used in services
        int user_present; // 0 or 1 -> for the UserScheduleService
        double outdoor_temp; // for the WeatherService
        double light_level; // for the WeatherService
        double price_per_kWh; // for the EnergyPriceService
    };

    class DataManager
    {
    public:
        DataManager(std::string filePath);
        std::vector<DataPoint> getAllData() const;
        // Functions to call on each service when they need the data
        DataPoint getDataPointForTime(uint32_t time) const;

    private:
        void loadDataFromCSV(const std::string &filePath);
        int getIndexForTime(uint32_t time) const;

        std::vector<DataPoint> _dataPoints;
    };

} // namespace POLA::Simulation