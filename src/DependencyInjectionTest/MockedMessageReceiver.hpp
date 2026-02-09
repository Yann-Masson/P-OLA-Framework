/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** MockedMessageReceiver
*/

#pragma once
#include "AMessageReceiver.hpp"

class MockedMessageReceiver : public AMessageReceiver
{
public:
    using AMessageReceiver::AMessageReceiver;
    void receiveMessage(const std::string &message) override;
};
