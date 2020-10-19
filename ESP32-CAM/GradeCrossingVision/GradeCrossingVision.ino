#include <esp_camera.h>
#include <esp_http_server.h>
#include <esp_timer.h>
#include <img_converters.h>
#include <fb_gfx.h>
#include <FS.h>
#include <SD_MMC.h>
#include <WiFi.h>
#include <String.h>
#include <Preferences.h>
#include <ArduinoOTA.h>

#include "common.h"
#include "camera_task.h"
#include "web_task.h"

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

    char buf64[64];
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
        // Reminder that data access will temporarily activate GPIO04 /
        // HS2_Data1 on which the flash LED is connected.
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

    Serial.printf("Wifi ssid: %s\n", gPrefWifiSsid.c_str());
    //--(debug only)--Serial.printf("Wifi pass: %s\n", gPrefWifiPass.c_str());
}

// ==== OTA ====

#define _OTA_ENABLED 0

#define _OTA_UNSET 0
#define _OTA_INIT 1
#define _OTA_IDLE 2
#define _OTA_UPDATE 3
int gOtaState = _OTA_UNSET;

bool is_ota_updating() {
    return gOtaState == _OTA_UPDATE;
}

void _ota_init() {
    Serial.println("[OTA] Init");
    // Source:
    // https://github.com/espressif/arduino-esp32/blob/master/libraries/ArduinoOTA/examples/BasicOTA/BasicOTA.ino

    ArduinoOTA.setPort(3232);  // Port defaults to 3232
    ArduinoOTA.setHostname("esp32-crossing-cam");
    // ArduinoOTA.setPassword("admin");
    ArduinoOTA.setRebootOnSuccess(true);

    ArduinoOTA
        .onStart([]() {
            String type;
            gOtaState = _OTA_UPDATE;
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "sketch";
            else  // U_SPIFFS
                type = "filesystem";

            // NOTE: if updating SPIFFS this would be the place to unmount
            // SPIFFS using SPIFFS.end()
            Serial.println("[OTA] Start updating " + type);
        })
        .onEnd([]() {
            Serial.println("\n[OTA] End");
            gOtaState = _OTA_IDLE;
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            Serial.printf("[OTA] Progress: %u of %u\n", progress, total);
        })
        .onError([](ota_error_t error) {
            Serial.printf("[OTA] Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR)
                Serial.println("[OTA] Auth Failed");
            else if (error == OTA_BEGIN_ERROR)
                Serial.println("[OTA] Begin Failed");
            else if (error == OTA_CONNECT_ERROR)
                Serial.println("[OTA] Connect Failed");
            else if (error == OTA_RECEIVE_ERROR)
                Serial.println("[OTA] Receive Failed");
            else if (error == OTA_END_ERROR)
                Serial.println("[OTA] End Failed");
        });

    gOtaState = _OTA_INIT;
}

void _ota_begin() {
    if (gOtaState == _OTA_INIT && is_wifi_connected()) {
        gOtaState = _OTA_IDLE;
        Serial.println("[OTA] Begin");
        ArduinoOTA.begin();
    }
}

void _ota_loop() {
    if (gOtaState == _OTA_INIT) {
        _ota_begin();
    } else if (gOtaState > _OTA_INIT) {
        ArduinoOTA.handle();
    }
}

// ==== Setup/Loop ====

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.printf("Grade Crossing INO running on Core %d\n", xPortGetCoreID());

    // Setup pins
    pinMode(PIN_FLASH, OUTPUT);
    pinMode(PIN_LED, OUTPUT);

    _blink();
    _prefs_init();
    _prefs_read();
    _sd_init();
    _sd_read_config();
    camera_task_init();
    wifi_init(gPrefWifiSsid, gPrefWifiPass);
    #if _OTA_ENABLED
        _ota_init();
    #endif
}

int __led_blink_ms = 0;

void loop() {
    unsigned long currentMillis = millis();

    wifi_loop();
    _ota_loop();

    if (gOtaState != _OTA_UPDATE) {
        delay(100);
        if (currentMillis - __led_blink_ms >= 1000 /* ms */) {
            __led_blink_ms = currentMillis;
            _blink();
            cam_print_stats();
        }
    }
}
