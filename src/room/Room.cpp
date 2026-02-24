#include "Room.hpp"

Room::Room(dicnew::ServiceProviderRef provider, double startingTemperature) : _temperature(startingTemperature),
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
	if (_provider.has<TemperatureFactorRegistry>())
	{
		std::cout << "Simulating room with " << _provider.get<TemperatureFactorRegistry>()->getFactors().size() << " temperature factors." << std::endl;
		for (auto &service : _provider.get<TemperatureFactorRegistry>()->getFactors())
		{
			_temperature += service->simulate();
		}
	}
	else
	{
		std::cout << "No TemperatureFactorRegistry found in provider!" << std::endl;
	}
}
