/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** Window
*/

#pragma once
#include "ATemperatureFactor.hpp"

class Window : public ATemperatureFactor {
    public:
        using ATemperatureFactor::ATemperatureFactor;
        double simulate() override;
};
