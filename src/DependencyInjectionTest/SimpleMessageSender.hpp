/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** SimpleMessageSender
*/

#pragma once
#include "AMessageSender.hpp"

class SimpleMessageSender : public AMessageSender
{
public:
    using AMessageSender::AMessageSender;
    void sendMessage(const std::string &message) override;
};
