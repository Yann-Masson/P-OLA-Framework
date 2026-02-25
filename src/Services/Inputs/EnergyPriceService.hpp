/**
 * @file EnergyPriceService.hpp
 * @brief Service providing current energy price data.
 */

#pragma once

#include "AInputService.hpp"
#include "Common/DataTypes.hpp"

namespace POLA::Services::Inputs {

class EnergyPriceService : public AInputService<Common::EnergyPriceData>
{
public:
    using AInputService<Common::EnergyPriceData>::AInputService;
    Common::EnergyPriceData getInput() override;
};

} // namespace POLA::Services::Inputs

