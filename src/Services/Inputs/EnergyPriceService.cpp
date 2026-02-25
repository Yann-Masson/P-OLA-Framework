/**
 * @file EnergyPriceService.cpp
 * @brief Implementation of the energy price data service.
 */

#include "EnergyPriceService.hpp"

using namespace POLA::Common;
using namespace POLA::Services::Inputs;

EnergyPriceData EnergyPriceService::getInput()
{
    return {
        .pricePerKWh = 0.15
    };
}
