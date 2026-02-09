#include "SimpleMessageSender.hpp"

#include "IMessageReceiverInterface.hpp"

void SimpleMessageSender::sendMessage(const std::string &message)
{
    std::cout << "SimpleMessageSender: " << message << std::endl;

    auto receiver = _provider.get<IMessageReceiverInterface>();
    if (receiver)
    {
        receiver->receiveMessage("Message acknowledged -> " + message);
    }
}
