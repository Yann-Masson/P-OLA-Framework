#include "UserComfortService.hpp"

#include "Inputs/GPSService.hpp"
#include "Inputs/UserPreferenceService.hpp"
#include <cmath>

using namespace POLA::Services;
using namespace POLA::Interfaces;
using namespace POLA::Common;

UserComfortService::UserComfortService(const forge::ProviderRef& provider)
{
}

double UserComfortService::recordComfort(double indoorTemp)
{
	double diff = 0.0;
	auto userLocation = _provider.get<IInputService<GPSData>>()->getInput();
	if (userLocation.distanceKm != 0) {
		return 0.0;
	}
	_comfortRecords++;
	auto userPreference = _provider.get<IInputService<UserPreferenceData>>()->getInput();
	if (indoorTemp < userPreference.minTemperature) {
		diff = userPreference.minTemperature - indoorTemp;
	} else if (indoorTemp > userPreference.maxTemperature) {
		diff = indoorTemp - userPreference.maxTemperature;
	}
	double comfort = 100.0 * std::exp(-ALPHA * diff);
	_totalComfort += comfort;
	return comfort;
}

double UserComfortService::getUserComfort() const
{
	return _comfortRecords > 0 ? _totalComfort / _comfortRecords : 0.0;
}

void UserComfortService::reset()
{
	_totalComfort = 0.0;
	_comfortRecords = 0;
}
