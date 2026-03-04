/**
 * @file SmartThermostatNoGPS.hpp
 * @brief GPS-free smart thermostat controller.
 *
 * Mirrors SmartThermostat but builds an AIStateNoGPS (4 features, no GPS)
 * and dispatches to IAIModelNoGPS instead of IAIModel.
 * The original SmartThermostat remains completely unchanged.
 */

#pragma once

#include "Interfaces/ISmartThermostat.hpp"
#include <forge/provider.hpp>

#define DECIDE_DELAY 60

namespace POLA::Simulation
{
    class SmartThermostatNoGPS : public Interfaces::ISmartThermostat
    {
    public:
        explicit SmartThermostatNoGPS(const forge::ProviderRef& provider);

        void simulate(double currentTemp) override;
        void reset() override;

    private:
        double decide(double currentTemp);
        forge::ProviderRef _provider;
        uint32_t _totalElapsedTime = 0;
    };
} // namespace POLA::Simulation
