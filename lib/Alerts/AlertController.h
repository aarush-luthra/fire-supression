#ifndef ALERT_CONTROLLER_H
#define ALERT_CONTROLLER_H

#include <Arduino.h>
#include "../FSM/SystemState.h"

class AlertController {
private:
    uint8_t _buzzerPin;
    uint8_t _ledPin;
    SystemState _currentState;
    unsigned long _lastLedUpdate;
    unsigned long _lastBuzzerUpdate;
    bool _ledState;
    bool _buzzerState;

public:
    AlertController(uint8_t buzzerPin, uint8_t ledPin);
    void begin();
    void setState(SystemState state);
    void update(); // Call in loop
};

#endif
