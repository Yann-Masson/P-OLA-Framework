/**
 * @file IConsumptionService.hpp
 * @brief Interface for tracking energy consumption and associated costs.
 */

#pragma once

namespace POLA::Interfaces {

class IConsumptionService {
public:
    virtual ~IConsumptionService() = default;

    virtual void recordEnergy(double kWh) = 0;
    [[nodiscard]] virtual double getTotalEnergyKWh() const = 0;
    [[nodiscard]] virtual double getTotalCost() const = 0;
    virtual void reset() = 0;
};

} // namespace POLA::Interfaces
