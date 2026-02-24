/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** TemperatureFactorRegistry
*/

#pragma once

#include <memory>
#include <vector>

#include <dicnew/ServiceProvider.hpp>

#include "ITemperatureFactor.hpp"
#include "Heater.hpp"
#include "Wall.hpp"
#include "Window.hpp"

class TemperatureFactorRegistry
{
public:
    TemperatureFactorRegistry(dicnew::ServiceProviderRef provider);

    const std::vector<std::shared_ptr<ITemperatureFactor>> getFactors() const;

private:
    std::vector<std::shared_ptr<ITemperatureFactor>> _factors;
};
