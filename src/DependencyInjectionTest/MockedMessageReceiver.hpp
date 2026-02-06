/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** MockedMessageReceiver
*/

#pragma once
#include "IMessageReceiverInterface.hpp"

class MockedMessageReceiver: public IMessageReceiverInterface {
    public:
        void receiveMessage(const std::string& message) override;

    protected:
    private:
};
