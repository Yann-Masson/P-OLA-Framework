/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** ITemperatureFactor
*/

#pragma once

class ITemperatureFactor {
    public:
        virtual ~ITemperatureFactor() = default;

        virtual double simulate() = 0;

    protected:
    private:
};
