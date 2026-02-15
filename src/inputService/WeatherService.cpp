#include "WeatherService.hpp"

WeatherData WeatherService::getInput()
{
	return {
		.outTemperature = 20.0,
		.enlightment = 1000.0
	};
}
