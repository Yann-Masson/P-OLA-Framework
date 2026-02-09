#include "SimpleMessageReceiver.hpp"

void SimpleMessageReceiver::receiveMessage(const std::string &message)
{
    std::cout << "SimpleMessageReceiver: Message received: " << message << std::endl;
}
