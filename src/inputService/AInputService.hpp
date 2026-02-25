/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** AInputService
*/

#pragma once
#include "IInputService.hpp"
#include <forge/provider.hpp>

template <typename T>
class AInputService : public IInputService<T>
{
public:
    explicit AInputService(forge::ProviderRef provider)
        : _provider(provider)
    {
    }

protected:
    forge::ProviderRef _provider;
};
