#include <Arduino.h>
#include "Config.h"
#include "SensorManager.h"
#include "StatsEngine.h"
#include "RiskEngine.h"
#include "SystemState.h"
#include "AlertController.h"

// Instantiate Global Objects
SensorManager sensors(GAS_PIN, FLAME_PIN);
StatsEngine stats(STATS_WINDOW_SIZE); // 600 samples for 1 minute buffer
AlertController alerts(BUZZER_PIN, LED_PIN);

// Global Variables
unsigned long lastTick = 0;

void setup() {
    Serial.begin(115200);
    Serial.println("=== Adaptive Edge-Based Fire Risk System Starting ===");

    sensors.begin();
    alerts.begin();

    Serial.println("System Initialized. Learning Baseline...");
}

void loop() {
    unsigned long now = millis();

    // Run system tick at fixed rate
    if (now - lastTick >= SYSTEM_TICK_RATE_MS) { // 100ms
        lastTick = now;

        // 1. Read Sensors
        float gasValue = sensors.readGas();
        bool flameDetected = sensors.readFlame();

        // 2. Update Statistics
        stats.addSample(gasValue);

        // 3. Check Baseline Validity
        bool baselineReady = stats.isBaselineReady();
        float gasZScore = 0.0f;
        
        if (baselineReady) {
            gasZScore = stats.getZScore(gasValue);
        } else {
            // Log learning progress
            if (millis() % 2000 == 0) {
                Serial.print("Learning Baseline... ");
                Serial.print(stats.getMean()); // Showing current mean building up
                Serial.println();
            }
        }

        // 4. Calculate Risk
        float riskScore = RiskEngine::calculate(gasZScore, flameDetected);

        // 5. Determine State
        SystemState newState = StateMachine::determineState(riskScore, baselineReady);

        // 6. Update Alerts
        alerts.setState(newState);

        // 7. Data Logging
        Serial.print("Gas:"); Serial.print(gasValue);
        Serial.print(",ZScore:"); Serial.print(gasZScore);
        Serial.print(",Risk:"); Serial.print(riskScore);
        Serial.print(",Flame:"); Serial.print(flameDetected);
        Serial.print(",State:"); 
        switch(newState) {
            case SystemState::BOOTUP: Serial.print("BOOTUP"); break;
            case SystemState::SAFE: Serial.print("SAFE"); break;
            case SystemState::WARNING: Serial.print("WARNING"); break;
            case SystemState::HIGH_RISK: Serial.print("HIGH_RISK"); break;
            case SystemState::EMERGENCY: Serial.print("EMERGENCY"); break;
        }
        Serial.println();
    }

    // High frequency update for alerts (blinking/beeping logic)
    alerts.update();
}
