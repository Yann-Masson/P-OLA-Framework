/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** MessageReceiverInterface
*/

#pragma once
#include <string>
#include <iostream>

class IMessageReceiverInterface
{
public:
    virtual ~IMessageReceiverInterface() = default;
    virtual void receiveMessage(const std::string &message) = 0;

protected:
private:
};
