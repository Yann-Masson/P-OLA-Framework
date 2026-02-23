/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** Heater
*/

#pragma once
#include "ATemperatureFactor.hpp"
#include "../consumptionService/IConsumptionService.hpp"

class Heater: public ATemperatureFactor {
    public:
        using ATemperatureFactor::ATemperatureFactor;
        double simulate() override;

        void setWantedTemperature(double wantedTemperature);

    private:
        double _wantedTemperature;
};
