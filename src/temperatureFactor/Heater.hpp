/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** Heater
*/

#pragma once
#include "ATemperatureFactor.hpp"

class Heater : public ATemperatureFactor
{
public:
    using ATemperatureFactor::ATemperatureFactor;
    double simulate(double insideTemperature) override;

    // TODO: find a way to call this function (because stored as a ITemperatureFactor)
    void setWantedTemperature(double wantedTemperature);

private:
    double _wantedTemperature;
};
