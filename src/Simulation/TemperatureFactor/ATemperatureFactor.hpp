/**
 * @file ATemperatureFactor.hpp
 * @brief Abstract base class for temperature factors affecting room temperature.
 */

#pragma once

#include "Interfaces/ITemperatureFactor.hpp"
#include <forge/provider.hpp>

namespace POLA::Simulation::TemperatureFactor {

class ATemperatureFactor : public Interfaces::ITemperatureFactor {
public:
    explicit ATemperatureFactor(const forge::ProviderRef& provider);

protected:
    forge::ProviderRef _provider;
};

} // namespace POLA::Simulation::TemperatureFactor
