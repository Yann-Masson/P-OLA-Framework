/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** Room
*/

#pragma once
#include <dicnew/ServiceProvider.hpp>
#include "../temperatureFactor/TemperatureFactorRegistry.hpp"
#include <iostream>

class Room
{
public:
    Room(dicnew::ServiceProviderRef provider, double startingTemperature = 20.0);
    ~Room();

    double getTemperature() const;
    void simulate();

private:
    double _temperature;
    dicnew::ServiceProviderRef _provider;
};
