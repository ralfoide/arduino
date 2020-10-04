#include "esp_camera.h"
#include <FS.h>
#include <SD_MMC.h>
#include <WiFi.h>
#include <String.h>
#include <Preferences.h>

// Set camera model
#define CAMERA_MODEL_AI_THINKER   // for ESP32-CAM module

#include "camera_pins.h"

#define PIN_NVS_RESET 16
#define PIN_FLASH 4
#define PIN_LED 33

// Global settings
Preferences gPrefs;
String gPrefWifiSsid;
String gPrefWifiPass;
uint16_t gPrefPoint1[2] = { 0, 0 };
uint16_t gPrefPoint2[2] = { 0, 0 };
uint16_t gPrefPointC[2] = { 0, 0 };
#define PREF_WIFI_SSID "wifiSsid"
#define PREF_WIFI_PASS "wifiPass"

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

void _prefs_init() {
  pinMode(PIN_NVS_RESET, INPUT_PULLUP);
}


void _prefs_read() {
  if (digitalRead(PIN_NVS_RESET) == LOW) {
    Serial.println("Prefs: Clearing NVS");
    if (gPrefs.begin("crossing", /* readOnly= */ false)) {
      gPrefs.clear();
      gPrefs.end();
    } else {
      Serial.println("Prefs: Can't open for clearing");
    }
  }

  if (!gPrefs.begin("crossing", /* readOnly= */ true)) {
    Serial.println("Prefs: Can't open for reading");
  }

  char* buf64 = new char[64];
  buf64[63] = 0;

  // SSID max len is 32 characters.
  if (gPrefs.getString(PREF_WIFI_SSID, buf64, 32) > 0) {
    gPrefWifiSsid = buf64;
  }

  // WEP key is 16 chars, WPA2 is 63.
  if (gPrefs.getString(PREF_WIFI_PASS, buf64, 63) > 0) {
    gPrefWifiPass = buf64;
  }

  gPrefPoint1[0] = gPrefs.getUShort("p1x", 0);
  gPrefPoint1[0] = gPrefs.getUShort("p1y", 0);
  gPrefPoint2[0] = gPrefs.getUShort("p2x", 0);
  gPrefPoint2[0] = gPrefs.getUShort("p2y", 0);
  gPrefPointC[0] = gPrefs.getUShort("pCx", 0);
  gPrefPointC[0] = gPrefs.getUShort("pCy", 0);

  delete[] buf64;
  gPrefs.end();
}

void _prefs_updateString(const char *key, const String &str) {
  if (!gPrefs.begin("crossing", /* readOnly= */ false)) {
    Serial.println("Prefs: Can't open for writing");
  }

  if (gPrefs.putString(key, str.c_str()) == 0) {
    Serial.printf("Prefs: Failed to write %s\n", key);
  } else {
    Serial.printf("Prefs: Updated %s\n", key);
  }

  gPrefs.end();
}

// ==== SD Card ====

void _sd_init() {
}

void _sd_read_config() {
  if (SD_MMC.begin()) {
    // Reminder that data access will temporarily activate GPIO04 / HS2_Data1
    // on which the flash LED is connected.
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
    f.close();

    ssid.trim();
    if (ssid.length() > 0) {
      if (gPrefWifiSsid != ssid) {
        gPrefWifiSsid = ssid;
        _prefs_updateString(PREF_WIFI_SSID, gPrefWifiSsid);
      }
    }
    pass.trim();
    if (pass.length() > 0) {
      if (gPrefWifiPass != pass) {
        gPrefWifiPass = pass;
        _prefs_updateString(PREF_WIFI_PASS, gPrefWifiPass);
      }
    }

    SD_MMC.end();
  } else {
    Serial.println("SD: Card not found");
  }

  // Ensure flash LED is off after reading SD MMC
  pinMode(PIN_FLASH, OUTPUT);
  _flash(0);
}

// ==== Setup/Loop ====

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // Setup pins
  pinMode(PIN_FLASH, OUTPUT);
  pinMode(PIN_LED, OUTPUT);

  _blink();
  _prefs_init();
  _prefs_read();
  _sd_init();
  _sd_read_config();

  Serial.printf("Wifi ssid: %s\n", gPrefWifiSsid.c_str());
  Serial.printf("Wifi pass: %s\n", gPrefWifiPass.c_str());
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  Serial.println("Loop");
  _blink();
}

