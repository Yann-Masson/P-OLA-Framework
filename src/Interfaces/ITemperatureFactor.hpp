/**
 * @file ITemperatureFactor.hpp
 * @brief Interface for components that affect room temperature (walls, windows, heaters).
 */

#pragma once

namespace POLA::Interfaces {

class ITemperatureFactor
{
public:
    virtual ~ITemperatureFactor() = default;

    /**
     * Simulate the temperature factor's effect for the current time step.
     *
     * @param insideTemperature The current inside temperature in degrees Celsius.
     * @return The net heat loss (positive) or gain (negative) in Watts (Joules per second).
     */
    virtual double simulate(double insideTemperature) = 0;
};

} // namespace POLA::Interfaces
