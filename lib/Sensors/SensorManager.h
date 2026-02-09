#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>

class SensorManager {
private:
    uint8_t _gasPin;
    uint8_t _flamePin;
    float _gasSmooth;

public:
    SensorManager(uint8_t gasPin, uint8_t flamePin);
    void begin();
    float readGas();      // Returns smoothed analog value
    bool readFlame();     // Returns true if flame detected
};

#endif
