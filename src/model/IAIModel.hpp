#pragma once

struct State {
    double tempIn;
    double tempOut;
    double electricityPrice;
    double gpsDistance;
    double userVelocity;
    double targetTemp;
};

class IAIModel {
public:
    virtual ~IAIModel() = default;
    virtual double predict(const State& state) = 0;
};
