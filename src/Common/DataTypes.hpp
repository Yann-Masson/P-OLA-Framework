/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** DataTypes
*/

#pragma once

struct EnergyPriceData {
    double pricePerKWh;
};

struct WeatherData {
    double outTemperature;
    double sunlightIntensity; // in W/m²
};

struct UserPreferenceData {
    double minTemperature;
    double maxTemperature;
};

struct GPSData {
    double distanceKm;
    double velocityKmMin;
};
