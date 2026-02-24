/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** ATemperatureFactor
*/

#pragma once
#include "ITemperatureFactor.hpp"
#include <dicnew/ServiceProvider.hpp>

class ATemperatureFactor : public ITemperatureFactor {
    public:
        explicit ATemperatureFactor(dicnew::ServiceProviderRef provider);

    protected:
        dicnew::ServiceProviderRef _provider;
};
