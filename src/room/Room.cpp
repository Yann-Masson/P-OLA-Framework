#include "Room.hpp"

Room::Room(dic::ServiceProviderRef provider, double startingTemperature):
	_temperature(startingTemperature),
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
	for (auto &service : _provider.get<TemperatureFactorRegistry>()->getFactors()) {
		_temperature += service->simulate();
	}
}
