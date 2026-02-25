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

    virtual double simulate(double insideTemperature) = 0;
};

} // namespace POLA::Interfaces
