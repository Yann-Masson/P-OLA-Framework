/*
** EPITECH PROJECT, 2026
** P-OLA-Framework
** File description:
** SimpleMessageReceiver
*/

#pragma once
#include "IMessageReceiverInterface.hpp"

class SimpleMessageReceiver: public IMessageReceiverInterface {
    public:
        void receiveMessage(const std::string& message) override;

    protected:
    private:
};
