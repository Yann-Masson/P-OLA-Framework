#include "MockedMessageReceiver.hpp"

void MockedMessageReceiver::receiveMessage(const std::string &message)
{
    std::cout << "MockedMessageReceiver: Message received: " << message << std::endl;
}
