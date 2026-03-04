/**
 * @file AIRecorder.cpp
 * @brief Implementation of the AI recorder.
 */

#include "AIRecorder.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>

using namespace POLA::Simulation;

void AIRecorder::record(double timestamp, const POLA::Common::AIState &state,
                        double prediction, double actualPower, double reward) {
  _records.push_back({timestamp, state.tempIn, state.electricityPrice,
                      state.userDistanceKm, state.userVelocityKmMin,
                      state.weather, state.userPreferences, state.userSchedule,
                      prediction, actualPower, reward});
}

void AIRecorder::writeToCSV(const std::string &filepath) {
  std::ofstream file(filepath);

  if (!file.is_open()) {
    std::cerr << "Error: Could not open file " << filepath << " for writing."
              << std::endl;
    return;
  }

  // Write header
  file << "Timestamp,TempIn,ElectricityPrice,UserDistanceKm,UserVelocityKmMin";
  for (int i = 0; i < 6; ++i) {
    file << ",ForecastTemp_h" << i << ",ForecastSunlight_h" << i;
  }
  file << ",PrefMinTemp,PrefMaxTemp";
  for (int i = 0; i < 24; ++i) {
    file << ",UserPresent_h" << i;
  }
  file << ",AIPrediction,ActualPower,Reward\n";

  // Write data with fixed precision
  file << std::fixed << std::setprecision(6);
  for (const auto &record : _records) {
    file << record.timestamp << "," << record.tempIn << ","
         << record.electricityPrice << "," << record.userDistanceKm << ","
         << record.userVelocityKmMin;
    for (const auto &wp : record.weather.forecast) {
      file << "," << wp.outdoorTemp << "," << wp.sunlightLuxIntensity;
    }
    file << "," << record.userPreferences.minTemperature << ","
         << record.userPreferences.maxTemperature;
    for (const bool present : record.userSchedule.userPresent) {
      file << "," << (present ? 1 : 0);
    }
    file << "," << record.aiPrediction << "," << record.actualPower << ","
         << record.reward << "\n";
  }

  file.close();
  std::cout << "AI records written to " << filepath << " (" << _records.size()
            << " records)" << std::endl;
}

size_t AIRecorder::getRecordCount() const { return _records.size(); }

void AIRecorder::clear() { _records.clear(); }
