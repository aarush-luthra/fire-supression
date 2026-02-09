#include "RiskEngine.h"

float RiskEngine::calculate(float gasZScore, bool flameDetected) {
    if (flameDetected) {
        return 100.0f; // Immediate max risk
    }

    // Map Z-Score to Risk (0-100)
    // Z < 2.0: Normal noise -> Risk ~0-10
    // 2.0 < Z < 6.0: Scaling up
    // Z > 6.0: High likelihood of gas/smoke -> Risk > 80

    if (gasZScore <= 2.0f) {
        return constrain(gasZScore * 5.0f, 0.0f, 10.0f);
    } 
    else if (gasZScore <= 6.0f) {
        // Linear mapping from Z=2->10 to Z=6->80
        // Slope = (80 - 10) / (6 - 2) = 70 / 4 = 17.5
        return 10.0f + ((gasZScore - 2.0f) * 17.5f);
    } 
    else {
        // Z > 6.0
        // Cap at 99 (reserving 100 for Fire)
        float risk = 80.0f + ((gasZScore - 6.0f) * 5.0f);
        return constrain(risk, 80.0f, 99.0f);
    }
}
