/**
 * @file Room.hpp
 * @brief Simulated room whose temperature is affected by registered temperature factors.
 */

#pragma once

#include <forge/provider.hpp>

namespace POLA::Simulation {

class Room
{
public:
    explicit Room(const forge::ProviderRef& provider, double startingTemperature = 20.0);

    double getTemperature() const;
    void simulate();

private:
    double _temperature;
    forge::ProviderRef _provider;
};

} // namespace POLA::Simulation

