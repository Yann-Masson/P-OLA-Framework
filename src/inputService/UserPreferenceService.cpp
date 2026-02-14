#include "UserPreferenceService.hpp"

UserPreferenceData UserPreferenceService::getInput()
{
	return {
		.minTemperature = 22.0,
		.maxTemperature = 28.0
	};
}
