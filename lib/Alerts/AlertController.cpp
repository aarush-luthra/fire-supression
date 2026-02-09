#include "AlertController.h"

AlertController::AlertController(uint8_t buzzerPin, uint8_t ledPin) {
  _buzzerPin = buzzerPin;
  _ledPin = ledPin;
  _currentState = SystemState::BOOTUP;
  _lastLedUpdate = 0;
  _lastBuzzerUpdate = 0;
  _ledState = false;
  _buzzerState =
      false; // "false" means OFF (which is HIGH for active-low buzzer)
}

void AlertController::begin() {
  pinMode(_buzzerPin, OUTPUT);
  pinMode(_ledPin, OUTPUT);
  digitalWrite(_buzzerPin, HIGH); // Off (active low)
  digitalWrite(_ledPin, LOW);

  // Startup Beep
  digitalWrite(_buzzerPin, LOW); // Beep ON
  delay(200);
  digitalWrite(_buzzerPin, HIGH); // Beep OFF
}

void AlertController::setState(SystemState state) {
  if (_currentState != state) {
    _currentState = state;
    // Reset timers/states on transition
    _ledState = false;
    digitalWrite(_ledPin, LOW);
    digitalWrite(_buzzerPin, HIGH); // Silence
  }
}

void AlertController::update() {
  unsigned long now = millis();

  switch (_currentState) {
  case SystemState::BOOTUP:
    // Fast LED blink (100ms)
    if (now - _lastLedUpdate > 100) {
      _lastLedUpdate = now;
      _ledState = !_ledState;
      digitalWrite(_ledPin, _ledState ? HIGH : LOW);
    }
    break;

  case SystemState::SAFE:
    digitalWrite(_ledPin, LOW);
    digitalWrite(_buzzerPin, HIGH);
    break;

  case SystemState::WARNING:
    // Slow LED blink (1s)
    if (now - _lastLedUpdate > 1000) {
      _lastLedUpdate = now;
      _ledState = !_ledState;
      digitalWrite(_ledPin, _ledState ? HIGH : LOW);
    }
    break;

  case SystemState::HIGH_RISK:
    // Fast LED blink (200ms)
    if (now - _lastLedUpdate > 200) {
      _lastLedUpdate = now;
      _ledState = !_ledState;
      digitalWrite(_ledPin, _ledState ? HIGH : LOW);
    }
    // Slow Buzzer Beep (1s on/off)
    if (now - _lastBuzzerUpdate > 1000) {
      _lastBuzzerUpdate = now;
      _buzzerState = !_buzzerState;
      digitalWrite(_buzzerPin, _buzzerState ? LOW : HIGH); // LOW is ON
    }
    break;

  case SystemState::EMERGENCY:
    // Solid LED, Continuous Buzzer
    digitalWrite(_ledPin, HIGH);
    digitalWrite(_buzzerPin, LOW); // ON
    break;
  }
}
