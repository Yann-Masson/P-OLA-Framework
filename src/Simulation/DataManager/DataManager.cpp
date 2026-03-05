#include "DataManager.hpp"

using namespace POLA::Simulation;

DataManager::DataManager(std::string filePath)
{
    loadDataFromCSV(filePath);
}

void DataManager::loadDataFromCSV(const std::string &filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file " << filePath << std::endl;
        return;
    }

    std::string line, word;
    std::getline(file, line); // Ignore header

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::vector<std::string> row;
        int user_present = 0;

        while (std::getline(ss, word, ';'))
        {
            row.push_back(word);
        }

        // Mapping to the struc (CSV related)
        // timestamp;user_present;activity;outdoor_temp;humidity;light_level;day_of_week;hour_of_day;price_kWh;user_distance;user_velocity
        if (row.size() >= 11)
        {
            DataPoint dp;
            dp.timestamp = row[0];                // Index 0: timestamp
            dp.user_present = std::stoi(row[1]);  // Index 1: user_present
            dp.outdoor_temp = std::stod(row[3]);  // Index 3: outdoor_temp
            dp.light_level = std::stod(row[5]);   // Index 5: light_level
            dp.price_per_kWh = std::stod(row[8]); // Index 8: price_kWh
            dp.user_distance = std::stod(row[9]); // Index 9: user_distance
            dp.user_velocity = std::stod(row[10]); // Index 10: user_velocity
            _dataPoints.push_back(dp);
        } else {
            std::cerr << "Warning: Skipping malformed line in CSV: " << line << std::endl;
        }
    }
    std::cout << "[DataManager] Successfully loaded " << _dataPoints.size() << " data points from CSV." << std::endl;
}

std::vector<DataPoint> DataManager::getAllData() const
{
    return _dataPoints;
}

DataPoint DataManager::getDataPointForTime(uint32_t time) const
{
    int index = getIndexForTime(time);
    return _dataPoints[index];
}

int DataManager::getIndexForTime(uint32_t time) const
{
    return static_cast<int>(time / 3600.0 * 4) % _dataPoints.size(); // Assuming data is collected every 15 minutes (4 data points per hour)
}
