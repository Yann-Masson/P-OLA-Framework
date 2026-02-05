//
// Created by Yann on 05/02/2026.
//

#ifndef P_OLA_FRAMEWORK_THERMALMODEL_HPP
#define P_OLA_FRAMEWORK_THERMALMODEL_HPP


#include <iostream>

class ThermalModel {
private:
    // Physical Constants (Adjust based on house type)
    double C = 50000.0;    // Thermal Capacity (J/degC) - "How much heat the air/walls hold"
    double R = 0.02;       // Thermal Resistance (degC/W) - "How well the walls are insulated"
    double maxPower = 2000.0; // Max heater power in Watts

    double currentTemp;
    double energyConsumedJoules = 0.0;

public:
    ThermalModel(double startTemp) : currentTemp(startTemp) {}

    // The core physics step
    void update(double tempOut, double controlSignal, double dt) {
        // controlSignal is 0.0 to 1.0 (0% to 100% heater power)
        double Q_heater = controlSignal * maxPower;

        // Equation: dT/dt = ( (T_out - T_in)/R + Q_heater ) / C
        double dT = ((tempOut - currentTemp) / R + Q_heater) / C;

        currentTemp += dT * dt; // Euler integration
        energyConsumedJoules += Q_heater * dt;
    }

    double getTemp() const { return currentTemp; }
    double getTotalEnergyKWh() const { return energyConsumedJoules / 3600000.0; }
};


#endif //P_OLA_FRAMEWORK_THERMALMODEL_HPP