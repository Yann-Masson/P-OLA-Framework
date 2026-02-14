/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** IConsumptionService
*/

#pragma once

class IConsumptionService {
    public:
        virtual ~IConsumptionService() = default;

        virtual void recordEnergy(double kWh) = 0;
        virtual double getTotalEnergyKWh() const = 0;
        virtual double getTotalCost() const = 0;
        virtual void reset() = 0;

    protected:
    private:
};
