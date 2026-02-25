/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** Room
*/

#pragma once

#include <forge/provider.hpp>

class Room
{
public:
    Room(const forge::ProviderRef& provider, double startingTemperature = 20.0);
    ~Room();

    double getTemperature() const;
    void simulate();

private:
    double _temperature;
    forge::ProviderRef _provider;
};
