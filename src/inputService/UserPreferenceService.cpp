#include "UserPreferenceService.hpp"

UserPreferenceData UserPreferenceService::getInput()
{
	return {
		.preferredTemperature = 22.0,
		.isHome = true
	};
}
