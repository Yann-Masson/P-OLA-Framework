/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** SimpleMessageSender
*/

#pragma once
#include "IMessageSenderInterface.hpp"

class SimpleMessageSender: public IMessageSenderInterface {
    public:
        void sendMessage(const std::string& message) override;

    protected:
    private:
};
