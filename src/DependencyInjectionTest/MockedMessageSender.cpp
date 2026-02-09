#include "MockedMessageSender.hpp"

void MockedMessageSender::sendMessage(const std::string &message)
{
    std::cout << "MockedMessageSender: " << message << std::endl;
}
