/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** IClock
*/

#pragma once

class IClock
{
public:
    virtual ~IClock() = default;
    virtual void simulate() = 0;
    virtual double elapsedTimeSinceStart() const = 0;
    virtual double getElapsedTime() const = 0;
};
