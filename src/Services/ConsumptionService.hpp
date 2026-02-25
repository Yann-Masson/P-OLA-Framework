/**
 * @file ConsumptionService.hpp
 * @brief Service for recording and tracking energy consumption and costs.
 */

#pragma once

#include <forge/provider.hpp>

#include "Interfaces/IConsumptionService.hpp"

namespace POLA::Services {

class ConsumptionService : public Interfaces::IConsumptionService
{
public:
    explicit ConsumptionService(const forge::ProviderRef& provider);

    void recordEnergy(double kWh) override;
    [[nodiscard]] double getTotalEnergyKWh() const override;
    [[nodiscard]] double getTotalCost() const override;
    void reset() override;

private:
    forge::ProviderRef _provider;
    double _totalEnergyKWh = 0.0;
    double _totalCost = 0.0;
};

} // namespace POLA::Services
