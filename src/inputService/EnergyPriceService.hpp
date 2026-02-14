/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** EnergyPriceService
*/

#pragma once
#include "AInputService.hpp"
#include "dataTypes.hpp"

class EnergyPriceService: public AInputService<EnergyPriceData> {
    public:
        using AInputService<EnergyPriceData>::AInputService;
        EnergyPriceData getInput() override;

    protected:
    private:
};
