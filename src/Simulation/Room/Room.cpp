/**
 * @file Room.cpp
 * @brief Implementation of the simulated room.
 */

#include "Room.hpp"

#include <iostream>

#include "Interfaces/IClock.hpp"
#include "Interfaces/ITemperatureFactor.hpp"
#include "Simulation/SmartThermostat/SmartThermostat.hpp"
#include "Simulation/TemperatureFactor/Wall.hpp"


using namespace POLA::Interfaces;
using namespace POLA::Simulation;

Room::Room(const forge::ProviderRef &provider, const double startingTemperature)
    : _indoorTemp(startingTemperature), _provider(provider) {
  const auto walls = _provider.getAll<TemperatureFactor::Wall>();

  // For simplicity, we assume the room is a rectangular box with 4 walls
  if (walls.size() != 4) {
    std::cerr << "Error: Room requires exactly 4 walls, but " << walls.size()
              << " were provided." << std::endl;
    throw std::runtime_error("Invalid number of walls for Room simulation.");
  }

  // Check if one wall can fit to be the opposite wall (same width and height)
  const auto wall1 = walls[0];
  const auto wall2 = walls[1];
  const auto wall3 = walls[2];
  const auto wall4 = walls[3];

  if (wall1->_width == wall3->_width && wall1->_height == wall3->_height &&
      wall2->_width == wall4->_width && wall2->_height == wall4->_height &&
      wall1->_height == wall2->_height) {
    _volume = wall1->_width * wall1->_height * wall2->_width;
  } else {
    std::cerr << "Error: Walls do not form a valid rectangular room."
              << std::endl;
    throw std::runtime_error("Invalid wall dimensions for Room simulation.");
  }
}

double Room::getTemperature() const { return _indoorTemp; }

void Room::setTemperature(const double temperature) {
  _indoorTemp = temperature;
}

void Room::simulate() {
  const auto clock = _provider.get<IClock>();
  const auto deltaTimeSeconds = clock->getElapsedTime();

  // Let the thermostat make heating decisions before we calculate physics
  const auto thermostat = _provider.get<SmartThermostat>();
  thermostat->simulate(_indoorTemp);

  const auto factors = _provider.getAll<ITemperatureFactor>();
  auto totalHeatLossWatts = 0.0;
  for (auto &service : factors) {
    totalHeatLossWatts += service->simulate(_indoorTemp);
  }

  const auto totalJoulesLost =
      totalHeatLossWatts *
      deltaTimeSeconds; // Convert Watts to Joules over the time step

  const auto airMassKg = _volume * AIR_DENSITY;

  // We use a negative sign because losing Joules means the temperature drops
  const auto deltaTempCelsius =
      -(totalJoulesLost) / (airMassKg * AIR_SPECIFIC_HEAT);

  _indoorTemp += deltaTempCelsius;
}
