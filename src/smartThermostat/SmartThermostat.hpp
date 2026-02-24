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
#include "../clock/IClock.hpp"
#include "../temperatureFactor/Heater.hpp"

#define DECIDE_DELAY 1000 // TODO: change this value

class SmartThermostat: public ISmartThermostat {
    public:
        SmartThermostat(dic::ServiceProviderRef provider);

        void simulate(double currentTemp) override;

    private:
        double decide(double currentTemp);
        dic::ServiceProviderRef _provider;
};
