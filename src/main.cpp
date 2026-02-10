#include "AlertController.h"
#include "Config.h"
#include "RiskEngine.h"
#include "SensorManager.h"
#include "StatsEngine.h"
#include "SystemState.h"
#include "WebServerManager.h"
#include <Arduino.h>

// Instantiate Global Objects
SensorManager sensors(GAS_PIN, FLAME_PIN);
StatsEngine stats(STATS_WINDOW_SIZE, SYSTEM_TICK_RATE_MS);
AlertController alerts(BUZZER_PIN, LED_PIN);
WebServerManager webServer;

// Global Variables
unsigned long lastTick = 0;
int flamePersistence = 0; // Consecutive ticks with flame detected

void setup() {
  Serial.begin(115200);
  Serial.println("=== Adaptive Edge-Based Fire Risk System Starting ===");

  sensors.begin();
  alerts.begin();
  webServer.begin();

  Serial.println("System Initialized. Learning Baseline...");
}

void loop() {
  unsigned long now = millis();

  // Run system tick at fixed rate
  if (now - lastTick >= SYSTEM_TICK_RATE_MS) {
    lastTick = now;

    // 1. Read Sensors
    float gasValue = sensors.readGas();
    bool flameDetected = sensors.readFlame();

    // 2. Update Statistics
    stats.addSample(gasValue);

    // 3. Track Flame Persistence
    if (flameDetected) {
      flamePersistence++;
    } else {
      flamePersistence = 0;
    }

    // 4. Check Baseline Validity
    bool baselineReady = stats.isBaselineReady();
    float gasZScore = 0.0f;
    float gasRate = 0.0f;

    if (baselineReady) {
      gasZScore = stats.getZScore(gasValue);
      gasRate = stats.getRateOfChange();
    } else {
      // Log learning progress
      if (millis() % 2000 == 0) {
        Serial.print("Learning Baseline... ");
        Serial.print(stats.getMean());
        Serial.println();
      }
    }

    // 5. Calculate Risk (Multi-Feature Fusion)
    float riskScore = RiskEngine::calculate(gasZScore, gasRate, gasValue,
                                            flamePersistence, flameDetected);

    // 6. Determine State
    SystemState newState =
        StateMachine::determineState(riskScore, baselineReady);

    // 7. Update Alerts
    alerts.setState(newState);

    // 8. Determine state string
    const char* stateStr = "UNKNOWN";
    switch (newState) {
    case SystemState::BOOTUP:   stateStr = "BOOTUP"; break;
    case SystemState::SAFE:     stateStr = "SAFE"; break;
    case SystemState::WARNING:  stateStr = "WARNING"; break;
    case SystemState::HIGH_RISK: stateStr = "HIGH_RISK"; break;
    case SystemState::EMERGENCY: stateStr = "EMERGENCY"; break;
    }

    // 9. Data Logging (Serial)
    Serial.print("Gas:");
    Serial.print(gasValue);
    Serial.print(",ZScore:");
    Serial.print(gasZScore);
    Serial.print(",Trend:");
    Serial.print(gasRate);
    Serial.print(",Risk:");
    Serial.print(riskScore);
    Serial.print(",Flame:");
    Serial.print(flameDetected);
    Serial.print(",FlamePersist:");
    Serial.print(flamePersistence);
    Serial.print(",State:");
    Serial.print(stateStr);
    Serial.println();

    // 10. Send data to Web Dashboard
    String data = "Gas:" + String(gasValue) +
                  ",ZScore:" + String(gasZScore) +
                  ",Trend:" + String(gasRate) +
                  ",Risk:" + String(riskScore) +
                  ",Flame:" + String(flameDetected) +
                  ",FlamePersist:" + String(flamePersistence) +
                  ",State:" + String(stateStr);
    webServer.sendData(data.c_str());
  }

  // High frequency update for alerts (blinking/beeping logic)
  alerts.update();
}
