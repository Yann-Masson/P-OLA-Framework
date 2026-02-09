/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** MessageSenderInterface
*/

#pragma once
#include "IMessageSenderInterface.hpp"
#include <cppdic/ServiceProvider.hpp>

class AMessageSender: public IMessageSenderInterface
{
public:
    explicit AMessageSender(dic::ServiceProviderRef provider);
protected:
    dic::ServiceProviderRef _provider;
};
