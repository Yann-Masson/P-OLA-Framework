#include "TemperatureFactorRegistry.hpp"

TemperatureFactorRegistry::TemperatureFactorRegistry(dicnew::ServiceProviderRef provider)
    : _factors{provider.get<Heater>(), provider.get<Wall>(),
               provider.get<Window>()}
{
}

const std::vector<std::shared_ptr<ITemperatureFactor>> TemperatureFactorRegistry::getFactors() const
{
    return _factors;
}
