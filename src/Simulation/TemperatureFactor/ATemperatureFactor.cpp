/**
 * @file ATemperatureFactor.cpp
 * @brief Implementation of the abstract temperature factor base class.
 */

#include "ATemperatureFactor.hpp"

namespace POLA::Simulation::TemperatureFactor {

ATemperatureFactor::ATemperatureFactor(const forge::ProviderRef& provider) : _provider(provider)
{
}

} // namespace POLA::Simulation::TemperatureFactor
