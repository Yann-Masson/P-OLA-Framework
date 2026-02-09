/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** SimpleMessageReceiver
*/

#pragma once
#include "AMessageReceiver.hpp"

class SimpleMessageReceiver : public AMessageReceiver
{
public:
    using AMessageReceiver::AMessageReceiver;
    void receiveMessage(const std::string &message) override;
};
