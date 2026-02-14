/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** AInputService
*/

#pragma once
#include "IInputService.hpp"
#include <cppdic/ServiceProvider.hpp>

template<typename T>
class AInputService: public IInputService<T> {
    public:
        explicit AInputService(dic::ServiceProviderRef provider)
            : _provider(provider)
        {
        }

    protected:
        dic::ServiceProviderRef _provider;
};
