#include <Arduino.h>
const int LDR_PIN = 34;   // АЦП-вхід (ADC1, input-only) — AO фоторезистора
const int LED_PIN = 5;    // PWM-вихід на LED

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);   // 0..4095 (на ESP32 це й так дефолт)
}

void loop() {
  int raw  = analogRead(LDR_PIN);          // 0..4095, світліше -> більше
  int duty = map(raw, 0, 4095, 255, 0);   // менше світла -> яскравіший LED (нічник)
  analogWrite(LED_PIN, duty);             // analogWrite сам підніме апаратний PWM (LEDC); є вже в core 2.0.17

  Serial.print("ADC=");  Serial.print(raw);
  Serial.print("  duty="); Serial.println(duty);
  delay(100);
}