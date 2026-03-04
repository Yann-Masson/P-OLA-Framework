/**
 * @file SmartThermostat.hpp
 * @brief Smart thermostat controller that decides heating levels using AI or
 * rules.
 */

#pragma once

#include "Interfaces/ISmartThermostat.hpp"
#include <forge/provider.hpp>

#define DECIDE_DELAY 60

namespace POLA::Simulation {

class SmartThermostat : public Interfaces::ISmartThermostat {
public:
  explicit SmartThermostat(const forge::ProviderRef &provider);

  void simulate(double currentTemp) override;
  void reset() override;

private:
  double decide(double currentTemp);
  forge::ProviderRef _provider;
  uint32_t _totalElapsedTime = 0;
};

} // namespace POLA::Simulation
