/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** dataTypes
*/

#pragma once

struct EnergyPriceData {
    double pricePerKWh;
};

struct WeatherData {
    double outTemperature;
    double enlightment;
};

struct UserPreferenceData {
    double minTemperature;
    double maxTemperature;
};

struct GPSData {
    double distanceKm;
    double velocityKmMin;
};