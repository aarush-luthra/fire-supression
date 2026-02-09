// =======================================
// ESP32 FLAME + GAS DETECTION SYSTEM
// =======================================

#define FLAME_PIN 27
#define GAS_PIN   33
#define BUZZER_PIN 25
#define LED_PIN 26

void setup() {
  Serial.begin(9600);

  pinMode(FLAME_PIN, INPUT);
  pinMode(GAS_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  // Initial safe state
  digitalWrite(BUZZER_PIN, HIGH); // buzzer OFF (active LOW)
  digitalWrite(LED_PIN, LOW);

  Serial.println("Fire & Gas Detection System Started");
}

void loop() {
  int flameState = digitalRead(FLAME_PIN);
  int gasState   = digitalRead(GAS_PIN);

  // LOW = detected (for both sensors)
  if (flameState == LOW && gasState == LOW) {
    // FIRE + GAS (EMERGENCY)
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("🚨 EMERGENCY: FIRE + GAS DETECTED!");
  }
  else if (flameState == LOW) {
    // FIRE only
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("🔥 FIRE DETECTED!");
  }
  else if (gasState == LOW) {
    // GAS only
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("💨 GAS / SMOKE DETECTED!");
  }
  else {
    // SAFE
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, HIGH);
    Serial.println("Environment Safe");
  }

  delay(500);
}
