/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** ConsumptionService
*/

#pragma once

#include <forge/provider.hpp>

#include "IConsumptionService.hpp"

class ConsumptionService : public IConsumptionService
{
public:
    ConsumptionService(forge::ProviderRef provider);

    void recordEnergy(double kWh) override;
    double getTotalEnergyKWh() const override;
    double getTotalCost() const override;
    void reset() override;

private:
    forge::ProviderRef _provider;
    double _totalEnergyKWh = 0.0;
    double _totalCost = 0.0;
};
