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
                        double prediction, double actualPower, double reward)
{
    _records.push_back({timestamp,
                        state.tempIn,
                        state.tempOut,
                        state.electricityPrice,
                        state.gpsDistance,
                        state.userVelocity,
                        state.targetTemp,
                        prediction,
                        actualPower,
                        reward});
}

void AIRecorder::writeToCSV(const std::string &filepath)
{
    std::ofstream file(filepath);

    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file " << filepath << " for writing." << std::endl;
        return;
    }

    // Write header
    file << "Timestamp,TempIn,TempOut,ElectricityPrice,GPSDistance,UserVelocity,"
         << "TargetTemp,AIPrediction,ActualPower,Reward\n";

    // Write data with fixed precision
    file << std::fixed << std::setprecision(6);
    for (const auto &record : _records)
    {
        file << record.timestamp << ","
             << record.tempIn << ","
             << record.tempOut << ","
             << record.electricityPrice << ","
             << record.gpsDistance << ","
             << record.userVelocity << ","
             << record.targetTemp << ","
             << record.aiPrediction << ","
             << record.actualPower << ","
             << record.reward << "\n";
    }

    file.close();
    std::cout << "AI records written to " << filepath << " ("
              << _records.size() << " records)" << std::endl;
}

size_t AIRecorder::getRecordCount() const
{
    return _records.size();
}

void AIRecorder::clear()
{
    _records.clear();
}
