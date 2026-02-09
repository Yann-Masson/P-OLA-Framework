#include <iostream>

#include "OLA_Agent.hpp"
#include "ThermalModel.hpp"
#include <cppdic/ServiceProvider.hpp>
#include <cppdic/ServiceProviderBuilder.hpp>
#include "DependencyInjectionTest/IMessageSenderInterface.hpp"
#include "DependencyInjectionTest/SimpleMessageSender.hpp"
#include "DependencyInjectionTest/MockedMessageSender.hpp"
#include "DependencyInjectionTest/IMessageReceiverInterface.hpp"
#include "DependencyInjectionTest/SimpleMessageReceiver.hpp"
#include "DependencyInjectionTest/MockedMessageReceiver.hpp"

int main()
{

    // Code with dependency injection
    // here only one declaration of the dependencies, and the rest is handled by the DI container
    auto provider = dic::ServiceProviderBuilder()
                        .addService<IMessageSenderInterface, SimpleMessageSender>()
                        .addService<IMessageReceiverInterface, SimpleMessageReceiver>()
                        .build();

    // Mocked versions
    // auto provider = dic::ServiceProviderBuilder()
    //                         .addService<IMessageSenderInterface, MockedMessageSender>()
    //                         .addService<IMessageReceiverInterface, MockedMessageReceiver>()
    //                         .build();

    provider.get<IMessageSenderInterface>()->sendMessage("Hello from DI!");
    provider.get<IMessageReceiverInterface>()->receiveMessage("Test from DI!");

    // Old code
    // ThermalModel room(18.0);       // Start room at 18°C
    // OLA_Controller smartAgent;

    // double simTime = 0;            // Seconds
    // double dt = 60.0;              // 1 minute steps
    // double totalCost = 0.0;

    // // Simulation loop (e.g., simulate 24 hours)
    // for (int i = 0; i < 1440; ++i) {
    //     // 1. Get External Data (This could be your API calls)
    //     double currentPrice = 0.15; // $/kWh
    //     double tempOut = 5.0;       // Cold day

    //     // 2. Prepare the State for the AI
    //     State currentState = {
    //         room.getTemp(),
    //         tempOut,
    //         currentPrice,
    //         10.5,  // 10.5 km away
    //         1.2    // km/minute
    //     };

    //     // 3. AI makes a decision
    //     double action = smartAgent.decide(currentState);

    //     // 4. Update Physics
    //     double energyBefore = room.getTotalEnergyKWh();
    //     room.update(tempOut, action, dt);
    //     double energyAfter = room.getTotalEnergyKWh();

    //     // 5. Track Cost
    //     totalCost += (energyAfter - energyBefore) * currentPrice;

    //     simTime += dt;

    //     // Log results for your paper's graphs
    //     std::cout << "Time: " << i << "m | Temp: " << room.getTemp()
    //               << " | Cost: $" << totalCost << std::endl;
    // }

    // return 0;
}
