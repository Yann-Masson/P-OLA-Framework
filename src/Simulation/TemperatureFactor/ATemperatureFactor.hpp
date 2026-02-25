/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** ATemperatureFactor
*/

#pragma once

#include "../../Interfaces/ITemperatureFactor.hpp"
#include <forge/provider.hpp>

class ATemperatureFactor : public ITemperatureFactor {
    public:
        explicit ATemperatureFactor(forge::ProviderRef provider);

    protected:
        forge::ProviderRef _provider;
};
