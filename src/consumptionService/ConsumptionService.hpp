/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** ConsumptionService
*/

#pragma once
#include "IConsumptionService.hpp"
#include <cppdic/ServiceProvider.hpp>

class ConsumptionService: public IConsumptionService {
    public:
        ConsumptionService(dic::ServiceProviderRef provider);

        void recordEnergy(double kWh) override;
        double getTotalEnergyKWh() const override;
        double getTotalCost() const override;
        void reset() override;

    private:
        dic::ServiceProviderRef _provider;
        double _totalEnergyKWh = 0.0;
        double _totalCost = 0.0;
};
