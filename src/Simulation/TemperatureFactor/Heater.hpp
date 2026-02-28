/**
 * @file Heater.hpp
 * @brief Heater temperature factor that increases room temperature based on a target.
 */

#pragma once

#include "ATemperatureFactor.hpp"

namespace POLA::Simulation::TemperatureFactor {

class Heater : public ATemperatureFactor
{
public:
    using ATemperatureFactor::ATemperatureFactor;
    double simulate(double insideTemperature) override;

    /// Set heater power level (0.0 = off, 1.0 = full power).
    void setPower(double power);

    /// Get current heater power level [0, 1].
    [[nodiscard]] double getPower() const;

private:
    double _power = 0.0; ///< Heater power level [0, 1]
};

} // namespace POLA::Simulation::TemperatureFactor
