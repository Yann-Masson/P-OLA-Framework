#include <iostream>
#include <torch/torch.h>

#include <dicnew/ServiceProvider.hpp>
#include <dicnew/ServiceProviderBuilder.hpp>
#include "clock/IClock.hpp"
#include "clock/SimulationClock.hpp"
#include <torch/script.h>
#include "inputService/EnergyPriceService.hpp"
#include "inputService/WeatherService.hpp"
#include "inputService/GPSService.hpp"
#include "inputService/UserPreferenceService.hpp"
#include "consumptionService/ConsumptionService.hpp"
#include "temperatureFactor/ITemperatureFactor.hpp"
#include "temperatureFactor/Heater.hpp"
#include "temperatureFactor/Wall.hpp"
#include "temperatureFactor/Window.hpp"
#include "room/Room.hpp"

int main()
{
    const auto provider = dicnew::ServiceProviderBuilder()
                        .addService<IClock, SimulationClock>()
                        .addService<IInputService<EnergyPriceData>, EnergyPriceService>()
                        .addService<IInputService<WeatherData>, WeatherService>()
                        .addService<IInputService<GPSData>, GPSService>()
                        .addService<IInputService<UserPreferenceData>, UserPreferenceService>()
                        .addService<IConsumptionService, ConsumptionService>()
                        .addMultiService<ITemperatureFactor, Heater>()
                        .addMultiService<ITemperatureFactor, Wall>()
                        .addMultiService<ITemperatureFactor, Wall>()
                        .addMultiService<ITemperatureFactor, Wall>()
                        .addMultiService<ITemperatureFactor, Wall>()
                        .addMultiService<ITemperatureFactor, Window>()
                        .addService<Room>()
                        .build();

    std::cout << "Service Provider initialized with services:" << std::endl;
    std::cout << "Temperature factors registered:" << std::endl;
    for (const auto& factor : provider.getAll<ITemperatureFactor>()) {
        std::cout << " - " << typeid(*factor).name() << std::endl;
    }

    std::cout << "Simulation clock initialized at time: " << provider.get<IClock>()->getElapsedTime() << " seconds" << std::endl;
    std::cout << "Energy price service initialized with current price: $" << provider.get<IInputService<EnergyPriceData>>()->getInput().pricePerKWh << " per kWh" << std::endl;
    std::cout << "Weather service initialized with current temperature: " << provider.get<IInputService<WeatherData>>()->getInput().outTemperature << "°C" << std::endl;
    std::cout << "GPS service initialized with current location: (" << provider.get<IInputService<GPSData>>()->getInput().distanceKm  << " km)" << std::endl;
    std::cout << "User preference service initialized with preferred temperature: " << provider.get<IInputService<UserPreferenceData>>()->getInput().maxTemperature << "°C" << std::endl;
    std::cout << "Consumption service initialized with total energy: " << provider.get<IConsumptionService>()->getTotalEnergyKWh() << " kWh and total cost: $" << provider.get<IConsumptionService>()->getTotalCost() << std::endl;

    auto room = provider.get<Room>();

    return 0;
}
