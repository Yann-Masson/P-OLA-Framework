/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** ATemperatureFactor
*/

#pragma once
#include "ITemperatureFactor.hpp"
#include <cppdic/ServiceProvider.hpp>

class ATemperatureFactor : public ITemperatureFactor {
    public:
        explicit ATemperatureFactor(dic::ServiceProviderRef provider);

    protected:
        dic::ServiceProviderRef _provider;
};
