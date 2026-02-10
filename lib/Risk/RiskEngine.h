#ifndef RISK_ENGINE_H
#define RISK_ENGINE_H

#include <Arduino.h>
#include "../../include/Config.h"

class RiskEngine {
public:
    // Multi-feature weighted risk calculation (0-100)
    // gasZScore: Deviation from baseline
    // gasRateOfChange: Rate of gas level change (units/sec)
    // gasAbsolute: Raw gas sensor reading
    // flamePersistence: Number of consecutive ticks flame detected
    // flameDetected: Current flame sensor state
    static float calculate(float gasZScore, float gasRateOfChange,
                           float gasAbsolute, int flamePersistence,
                           bool flameDetected);

private:
    static float mapZScore(float z);
    static float mapTrend(float rate);
    static float mapAbsolute(float raw);
    static float mapFlamePersistence(int ticks);
};

#endif
