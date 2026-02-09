/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** MockedMessageSender
*/

#pragma once
#include "AMessageSender.hpp"

class MockedMessageSender : public AMessageSender
{
public:
    using AMessageSender::AMessageSender;
    void sendMessage(const std::string &message) override;
};
