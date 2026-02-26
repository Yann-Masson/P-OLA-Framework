/**
 * @file ISmartThermostat.hpp
 * @brief Interface for the smart thermostat controller.
 */

#pragma once

namespace POLA::Interfaces {

class ISmartThermostat {
public:
    virtual ~ISmartThermostat() = default;
    virtual void simulate(double currentTemp) = 0;
};

} // namespace POLA::Interfaces
