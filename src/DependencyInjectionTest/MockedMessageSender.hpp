/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** MockedMessageSender
*/

#pragma once
#include "IMessageSenderInterface.hpp"

class MockedMessageSender: public IMessageSenderInterface {
    public:
        void sendMessage(const std::string& message) override;

    protected:
    private:
};
