#ifndef RISK_ENGINE_H
#define RISK_ENGINE_H

#include <Arduino.h>

class RiskEngine {
public:
    // Calculate risk score (0-100) based on inputs
    // gasZScore: Deviation from baseline
    // flameDetected: Binary flame sensor input
    static float calculate(float gasZScore, bool flameDetected);
};

#endif
