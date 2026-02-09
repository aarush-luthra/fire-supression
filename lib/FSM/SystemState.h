#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

enum class SystemState {
    BOOTUP,
    SAFE,
    WARNING,
    HIGH_RISK,
    EMERGENCY
};

class StateMachine {
public:
    static SystemState determineState(float riskScore, bool baselineReady) {
        if (!baselineReady) return SystemState::BOOTUP;
        
        if (riskScore >= 100.0f) return SystemState::EMERGENCY; // Fire detected
        if (riskScore >= 80.0f) return SystemState::HIGH_RISK;  // Significant gas/smoke
        if (riskScore >= 40.0f) return SystemState::WARNING;    // Elevated levels
        return SystemState::SAFE;
    }
};

#endif
