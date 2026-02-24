/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** SmartThermostat
*/

#pragma once
#include "ISmartThermostat.hpp"
#include <cppdic/ServiceProvider.hpp>
#include "../inputService/DataTypes.hpp"
#include "../inputService/IInputService.hpp"

class SmartThermostat: public ISmartThermostat {
    public:
        SmartThermostat(dic::ServiceProviderRef provider);

        double decide(double currentTemp) override;

    private:
        dic::ServiceProviderRef _provider;
};
