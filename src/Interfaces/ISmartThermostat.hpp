/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** ISmartThermostat
*/

#pragma once

class ISmartThermostat {
    public:
        virtual ~ISmartThermostat() = default;
        virtual void simulate(double currentTemp) = 0;
};
