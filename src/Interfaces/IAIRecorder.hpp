/**
 * @file IAIRecorder.hpp
 * @brief Interface for AI state and prediction recording service.
 */

#pragma once

#include "Common/AIState.hpp"
#include <string>

namespace POLA::Interfaces
{

    /**
     * @brief Interface for recording AI states and predictions
     */
    class IAIRecorder
    {
    public:
        virtual ~IAIRecorder() = default;

        /**
         * @brief Record an AI state and its prediction
         */
        virtual void record(double timestamp, const POLA::Common::AIState &state,
                            double prediction, double actualPower, double reward) = 0;

        /**
         * @brief Write all records to a CSV file
         */
        virtual void writeToCSV(const std::string &filepath) = 0;

        /**
         * @brief Get the number of records
         */
        virtual size_t getRecordCount() const = 0;

        /**
         * @brief Clear all records
         */
        virtual void clear() = 0;
    };

} // namespace POLA::Interfaces
