/**
 * @file EnergyPriceService.hpp
 * @brief Service providing current energy price data.
 */

#pragma once

#include "AInputService.hpp"
#include "Common/DataTypes.hpp"
#include "Simulation/DataManager/DataManager.hpp"
#include "Interfaces/IClock.hpp"

namespace POLA::Services::Inputs {

class EnergyPriceService : public AInputService<Common::EnergyPriceData>
{
public:
    using AInputService<Common::EnergyPriceData>::AInputService;
    Common::EnergyPriceData getInput() override;
private:
    static constexpr int PRICES_LENGTH = 6; // Number of hours to provide price forecast for
};

} // namespace POLA::Services::Inputs

