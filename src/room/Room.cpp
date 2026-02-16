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
	// TODO: call all of the temperature factor services and update the temperature accordingly
	// for (auto &service : _provider.get<ITemperatureFactor>().getAll()) {
	// 	_temperature += service->simulate();
	// }
}
