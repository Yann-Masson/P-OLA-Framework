/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** ConsumptionService
*/

#pragma once
#include "IConsumptionService.hpp"
#include <dicnew/ServiceProvider.hpp>
#include "../inputService/IInputService.hpp"
#include "../inputService/DataTypes.hpp"

class ConsumptionService : public IConsumptionService
{
public:
    ConsumptionService(dicnew::ServiceProviderRef provider);

    void recordEnergy(double kWh) override;
    double getTotalEnergyKWh() const override;
    double getTotalCost() const override;
    void reset() override;

private:
    dicnew::ServiceProviderRef _provider;
    double _totalEnergyKWh = 0.0;
    double _totalCost = 0.0;
};
