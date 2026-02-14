/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** EnergyPriceService
*/

#pragma once
#include "AInputService.hpp"

struct EnergyPriceData {
    double pricePerKWh;
};

class EnergyPriceService: public AInputService<EnergyPriceData> {
    public:
        using AInputService<EnergyPriceData>::AInputService;
        EnergyPriceData getInput() override;

    protected:
    private:
};
