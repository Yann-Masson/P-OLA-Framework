#include "Room.hpp"

#include <iostream>

#include "../temperatureFactor/ITemperatureFactor.hpp"

Room::Room(const forge::ProviderRef& provider, const double startingTemperature) : _temperature(startingTemperature),
                                                                      _provider(provider)
{
}

Room::~Room()
{
}

double Room::getTemperature() const
{
	return _temperature;
}

void Room::simulate()
{
	auto factors = _provider.getAll<ITemperatureFactor>();
	if (!factors.empty())
	{
		std::cout << "Simulating room with " << factors.size() << " temperature factors." << std::endl;
		for (auto &service : factors)
		{
			_temperature += service->simulate();
		}
	}
	else
	{
		std::cout << "No temperature factors found in provider!" << std::endl;
	}
}
