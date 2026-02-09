#include "SensorManager.h"

SensorManager::SensorManager(uint8_t gasPin, uint8_t flamePin) {
    _gasPin = gasPin;
    _flamePin = flamePin;
    _gasSmooth = 0;
}

void SensorManager::begin() {
    pinMode(_gasPin, INPUT);
    pinMode(_flamePin, INPUT);
}

float SensorManager::readGas() {
    int raw = analogRead(_gasPin);
    // Simple exponential smoothing
    // alpha = 0.2 (20% new value, 80% old value)
    if (_gasSmooth == 0) _gasSmooth = raw; // Init
    else _gasSmooth = (_gasSmooth * 0.8) + (raw * 0.2);
    return _gasSmooth;
}

bool SensorManager::readFlame() {
    // Flame sensor is active LOW (LOW = Fire)
    return digitalRead(_flamePin) == LOW;
}
