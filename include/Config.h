#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Pin Definitions
#define FLAME_PIN 27
#define GAS_PIN   33
#define BUZZER_PIN 25
#define LED_PIN 26

// Sensor Configuration
#define GAS_SAMPLE_INTERVAL_MS 100 // Sample gas every 100ms
#define STATS_WINDOW_SIZE 600      // Moving average over 1 minute (60s / 0.1s)
#define BASELINE_LEARN_TIME_MS 60000 // 1 minute to learn baseline

// Risk Configuration
#define RISK_SCORE_MAX 100.0f
#define Z_SCORE_WARNING_THRESHOLD 3.0f   // Statistically significant deviation
#define Z_SCORE_HIGHRISK_THRESHOLD 6.0f  // Very high deviation

// System Update Rate
#define SYSTEM_TICK_RATE_MS 50     // Main loop delay

#endif
