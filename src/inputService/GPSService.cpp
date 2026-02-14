#include "GPSService.hpp"

GPSData GPSService::getInput()
{
	return {
		.distanceKm = 10.0,
		.velocityKmMin = 60.0
	};
}
