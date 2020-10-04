#include "esp_camera.h"
#include <FS.h>
#include <SD_MMC.h>
#include <WiFi.h>

// Set camera model
#define CAMERA_MODEL_AI_THINKER   // for ESP32-CAM module

#include "camera_pins.h"

#define PIN_FLASH 4
#define PIN_LED 33


// ==== Utilities ====

void _blink() {
  digitalWrite(PIN_LED, LOW);
  delay(100 /*ms*/);
  digitalWrite(PIN_LED, HIGH);
}

void _flash(int durationMs) {
  if (durationMs > 0) {
    digitalWrite(PIN_FLASH, HIGH);
    delay(durationMs /*ms*/);
  }
  digitalWrite(PIN_FLASH, LOW);
}

// ==== SD Card ====

void _sd_init() {
  if (!SD_MMC.begin()) {
    Serial.println("SD: Card not found");
  }
}

void _sd_read_config() {
  uint8_t card_type = SD_MMC.cardType();
  if (card_type == CARD_NONE) {
    return;
  }
  File f = SD_MMC.open("/config.txt");
  if (!f) {
    Serial.println("SD: Config file not found.");
    return;
  }
  String ssid = f.readStringUntil('\n');
  String pass = f.readStringUntil('\n');
  Serial.print("Wifi ssid: ");
  Serial.println(ssid);
  Serial.print("Wifi pass: ");
  Serial.println(pass);
  f.close();

  // Ensure flash LED is off after reading SD MMC
  pinMode(PIN_FLASH, OUTPUT);
  _flash(0);
}

// ==== Setup/Loop ====

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // Setup LED & Flash
  pinMode(PIN_FLASH, OUTPUT);
  pinMode(PIN_LED, OUTPUT);

  _blink();
  _sd_init();
  _sd_read_config();
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  Serial.println("Loop");
  _blink();
}

