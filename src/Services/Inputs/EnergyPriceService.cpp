#include "../../inputService/EnergyPriceService.hpp"

EnergyPriceData EnergyPriceService::getInput()
{
	return {
		.pricePerKWh = 0.15
	};
}
