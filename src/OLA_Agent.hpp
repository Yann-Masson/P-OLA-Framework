//
// Created by Yann on 05/02/2026.
//

#ifndef P_OLA_SIMULATOR_OLA_AGENT_HPP
#define P_OLA_SIMULATOR_OLA_AGENT_HPP

struct State {
    double tempIn;
    double tempOut;
    double electricityPrice;
    double gpsDistance;
    double userVelocity;
};

class OLA_Controller {
public:
    // This is where your LibTorch / Neural Network logic will go
    double decide(State s) {
        // Logic Example:
        // If TTA (Time to Arrival) < time needed to heat, then 1.0, else 0.0
        double tta = s.gpsDistance / s.userVelocity;

        // For your paper: This 'decide' function will eventually be
        // return neural_net.forward(state_tensor);
        if (tta < 0.5 && s.tempIn < 20.0) return 1.0;

        return 0.0;
    }
};



#endif //P_OLA_SIMULATOR_OLA_AGENT_HPP