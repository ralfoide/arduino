#include "esp_camera.h"
#include <WiFi.h>

// Set camera model
#define CAMERA_MODEL_AI_THINKER   // for ESP32-CAM module

#include "camera_pins.h"

#define PIN_FLASH 4
#define PIN_LED 33

void _blink() {
  digitalWrite(PIN_LED, LOW);
  delay(100 /*ms*/);
  digitalWrite(PIN_LED, HIGH);
}

void _flash(int durationMs) {
  digitalWrite(PIN_FLASH, HIGH);
  delay(durationMs /*ms*/);
  digitalWrite(PIN_FLASH, LOW);
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // Setup LED & Flash
  pinMode(PIN_FLASH, OUTPUT);
  pinMode(PIN_LED, OUTPUT);

  _blink();
  _flash(100);
  _blink();
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  Serial.print("Loop");
  _blink();
}

