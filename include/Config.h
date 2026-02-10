#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Wi-Fi Configuration
// Default to Access Point mode if no credentials provided
#define WIFI_SSID "ESP32_Fire_System" // Default AP Name
#define WIFI_PASS "12345678"          // Default AP Password
#define USE_AP_MODE                                                            \
  true // Set to false for Station mode (connect to home Wi-Fi)

// Web Server Configuration
#define WEB_SERVER_PORT 80

// Pin Definitions
#define FLAME_PIN 27
#define GAS_PIN 33
#define BUZZER_PIN 25
#define LED_PIN 26

// Sensor Configuration
#define GAS_SAMPLE_INTERVAL_MS 100   // Sample gas every 100ms
#define STATS_WINDOW_SIZE 60         // Moving average over 30s (60 * 0.5s)
#define BASELINE_LEARN_TIME_MS 60000 // 1 minute to learn baseline

// Risk Configuration
#define RISK_SCORE_MAX 100.0f
#define Z_SCORE_WARNING_THRESHOLD 3.0f  // Statistically significant deviation
#define Z_SCORE_HIGHRISK_THRESHOLD 6.0f // Very high deviation

// Gas Absolute Level Thresholds (raw ADC values)
#define GAS_ABSOLUTE_SAFE 800       // Below this = normal environment
#define GAS_ABSOLUTE_DANGER 2500    // Above this = dangerous regardless of baseline

// Trend Detection
#define GAS_TREND_DANGER 50.0f      // Units/sec rise rate for max trend score

// Flame Persistence
#define FLAME_PERSIST_THRESHOLD 3   // Ticks of sustained flame = confirmed fire

// Risk Weights (must sum to 1.0)
#define RISK_WEIGHT_ZSCORE   0.35f
#define RISK_WEIGHT_TREND    0.25f
#define RISK_WEIGHT_ABSOLUTE 0.20f
#define RISK_WEIGHT_FLAME    0.20f

// System Update Rate
#define SYSTEM_TICK_RATE_MS 500 // Main loop delay

#endif
