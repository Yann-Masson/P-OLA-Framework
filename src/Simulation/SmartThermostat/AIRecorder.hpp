/**
 * @file AIRecorder.hpp
 * @brief Records AI state and predictions for analysis.
 */

#pragma once

#include "Common/AIState.hpp"
#include "Interfaces/IAIRecorder.hpp"
#include <vector>
#include <string>

namespace POLA::Simulation
{

    struct AIRecord
    {
        double timestamp;
        double tempIn;
        double tempOut;
        double electricityPrice;
        double gpsDistance;
        double userVelocity;
        double targetTemp;
        double aiPrediction;
        double actualPower;
        double reward;
    };

    class AIRecorder : public POLA::Interfaces::IAIRecorder
    {
    public:
        AIRecorder() = default;

        /**
         * @brief Record an AI state and its prediction
         */
        void record(double timestamp, const POLA::Common::AIState &state,
                    double prediction, double actualPower, double reward) override;

        /**
         * @brief Write all records to a CSV file
         */
        void writeToCSV(const std::string &filepath) override;

        /**
         * @brief Get the number of records
         */
        size_t getRecordCount() const override;

        /**
         * @brief Clear all records
         */
        void clear() override;

    private:
        std::vector<AIRecord> _records;
    };

} // namespace POLA::Simulation
