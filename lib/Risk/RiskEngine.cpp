#include "RiskEngine.h"

float RiskEngine::calculate(float gasZScore, float gasRateOfChange,
                            float gasAbsolute, int flamePersistence,
                            bool flameDetected) {
    // Sustained flame = confirmed fire → immediate max risk
    if (flameDetected && flamePersistence >= FLAME_PERSIST_THRESHOLD) {
        return 100.0f;
    }

    // Weighted multi-feature fusion
    float zComponent    = mapZScore(gasZScore)          * RISK_WEIGHT_ZSCORE;
    float trendComp     = mapTrend(gasRateOfChange)     * RISK_WEIGHT_TREND;
    float absoluteComp  = mapAbsolute(gasAbsolute)      * RISK_WEIGHT_ABSOLUTE;
    float flameComp     = mapFlamePersistence(flamePersistence) * RISK_WEIGHT_FLAME;

    float risk = zComponent + trendComp + absoluteComp + flameComp;
    return constrain(risk, 0.0f, 99.0f);
}

// Map Z-Score to 0-100 (same curve as original)
float RiskEngine::mapZScore(float z) {
    if (z <= 0.0f) return 0.0f;
    if (z <= 2.0f) {
        return constrain(z * 5.0f, 0.0f, 10.0f);
    } else if (z <= 6.0f) {
        return 10.0f + ((z - 2.0f) * 17.5f);
    } else {
        float score = 80.0f + ((z - 6.0f) * 5.0f);
        return constrain(score, 80.0f, 100.0f);
    }
}

// Map rate of change to 0-100
// Positive rate = gas rising = danger
// Negative rate = gas falling = safe
float RiskEngine::mapTrend(float rate) {
    if (rate <= 0.0f) return 0.0f; // Falling or stable = no trend risk
    // Linear map: 0 → 0, GAS_TREND_DANGER → 100
    float score = (rate / GAS_TREND_DANGER) * 100.0f;
    return constrain(score, 0.0f, 100.0f);
}

// Map raw gas value to 0-100
float RiskEngine::mapAbsolute(float raw) {
    if (raw <= GAS_ABSOLUTE_SAFE) return 0.0f;
    if (raw >= GAS_ABSOLUTE_DANGER) return 100.0f;
    // Linear interpolation between safe and danger
    return ((raw - GAS_ABSOLUTE_SAFE) / (GAS_ABSOLUTE_DANGER - GAS_ABSOLUTE_SAFE)) * 100.0f;
}

// Map flame persistence to 0-100
// 0 ticks = 0, FLAME_PERSIST_THRESHOLD ticks = 100
float RiskEngine::mapFlamePersistence(int ticks) {
    if (ticks <= 0) return 0.0f;
    if (ticks >= FLAME_PERSIST_THRESHOLD) return 100.0f;
    return ((float)ticks / FLAME_PERSIST_THRESHOLD) * 100.0f;
}
