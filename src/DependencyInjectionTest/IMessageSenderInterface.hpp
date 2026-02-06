/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** MessageSenderInterface
*/

#pragma once
#include <string>
#include <iostream>

class IMessageSenderInterface
{
public:
    virtual ~IMessageSenderInterface() = default;
    virtual void sendMessage(const std::string &message) = 0;

protected:
private:
};
