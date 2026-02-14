/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** Wall
*/

#pragma once
#include "ATemperatureFactor.hpp"

class Wall : public ATemperatureFactor {
    public:
        using ATemperatureFactor::ATemperatureFactor;
        double simulate() override;
};
