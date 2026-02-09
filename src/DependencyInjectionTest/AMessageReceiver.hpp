/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** MessageReceiverInterface
*/

#pragma once
#include "IMessageReceiverInterface.hpp"
#include <cppdic/ServiceProvider.hpp>

class AMessageReceiver: public IMessageReceiverInterface
{
public:
    explicit AMessageReceiver(dic::ServiceProviderRef provider);

protected:
    dic::ServiceProviderRef _provider;
};
