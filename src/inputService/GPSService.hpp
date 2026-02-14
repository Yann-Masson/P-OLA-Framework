/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** GPSService
*/

#pragma once
#include "AInputService.hpp"

struct GPSData {
    double distanceKm;
    double velocityKmMin;
};

class GPSService: public AInputService<GPSData> {
    public:
        using AInputService<GPSData>::AInputService;
        GPSData getInput() override;

    protected:
    private:
};
