/**
 * @file EnergyPriceService.cpp
 * @brief Implementation of the energy price data service.
 */

#include "EnergyPriceService.hpp"

namespace POLA::Services::Inputs {

Common::EnergyPriceData EnergyPriceService::getInput()
{
    return {
        .pricePerKWh = 0.15
    };
}

} // namespace POLA::Services::Inputs
