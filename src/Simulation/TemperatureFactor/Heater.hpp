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

    void setWantedTemperature(double wantedTemperature);

private:
    double _wantedTemperature;
};

} // namespace POLA::Simulation::TemperatureFactor
