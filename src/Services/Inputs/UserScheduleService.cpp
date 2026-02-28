#include "UserScheduleService.hpp"

using namespace POLA::Common;
using namespace POLA::Services::Inputs;
using namespace POLA::Simulation;
using namespace POLA::Interfaces;

UserScheduleData UserScheduleService::getInput()
{
	auto dataManager = _provider.get<DataManager>();
	auto clock = _provider.get<IClock>();
	uint32_t currentTime = clock->getElapsedTimeSinceStart();
	std::vector<bool> userSchedule;
	for (int i = 0; i < 24; i++) {
		DataPoint dp = dataManager->getDataPointForTime(currentTime + i * 3600);
		userSchedule.push_back(dp.user_present == 1);
	}
	return UserScheduleData{userSchedule};
}
