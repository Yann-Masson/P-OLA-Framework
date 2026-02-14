/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** WeatherService
*/

#pragma once
#include "AInputService.hpp"
#include "dataTypes.hpp"

class WeatherService: public AInputService<WeatherData> {
    public:
        using AInputService<WeatherData>::AInputService;
        WeatherData getInput() override;

    protected:
    private:
};
