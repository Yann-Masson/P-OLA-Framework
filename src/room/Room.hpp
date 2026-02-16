/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** Room
*/

#pragma once
#include <cppdic/ServiceProvider.hpp>
#include "../temperatureFactor/ITemperatureFactor.hpp"

class Room {
    public:
        Room(dic::ServiceProviderRef provider, double startingTemperature = 20.0);
        ~Room();

        double getTemperature() const;
        void simulate();

    private:
        double _temperature;
        dic::ServiceProviderRef _provider;
};
