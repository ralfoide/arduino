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
#include "common.h"

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

  Serial.printf("Wifi ssid: %s\n", gPrefWifiSsid.c_str());
  Serial.printf("Wifi pass: %s\n", gPrefWifiPass.c_str());
}

// ==== Camera ====

void _camera_init() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  //init with high specs to pre-allocate larger buffers
  if (psramFound()) {
    Serial.println("Camera: PSRAM found. Using SVGA size.");
    // With 4 MB PSRAM we can support UXGA but it is overkill for our application.
    //--config.frame_size = FRAMESIZE_UXGA;
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    Serial.println("Camera: ERROR PSRAM not found.");
    return;
    // Serial.println("Camera: SVGA size");
    // config.frame_size = FRAMESIZE_SVGA;
    // config.jpeg_quality = 12;
    // config.fb_count = 1;
  }

// #if defined(CAMERA_MODEL_ESP_EYE)
//   // Not used on ESP32-CAM
//   pinMode(13, INPUT_PULLUP);
//   pinMode(14, INPUT_PULLUP);
// #endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera: init failed with error 0x%x\n", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  // Unused code, Our sensor is an OV2640.
  // // initial sensors are flipped vertically and colors are a bit saturated
  // if (s->id.PID == OV3660_PID) {
  //   Serial.println("Camera: OV3660 PID");
  //   s->set_vflip(s, 1);       //flip it back
  //   s->set_brightness(s, 1);  //up the blightness just a bit
  //   s->set_saturation(s, -2); //lower the saturation
  // }

  // // [RM] drop down frame size for higher initial frame rate
  // s->set_framesize(s, FRAMESIZE_QVGA);

// #if defined(CAMERA_MODEL_M5STACK_WIDE)
//   // Not used on ESP32-CAM
//   s->set_vflip(s, 1);
//   s->set_hmirror(s, 1);
// #endif

  // Depending on how the sensor is mounted, this can be useful.
  // TBD: Make it a startup preference.
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
}

// ==== HTTP Server ====

#define HTTP_PORT 80
httpd_handle_t gCameraHttpd = NULL;

uint32_t gStatImgCount = 0;
int64_t  gStatLastCaptureTs = 0;
uint32_t gStatDeltaGrabMs = 0;
uint32_t gStatDeltaSendMs = 0;


size_t _jpg_encode_stream(void * arg, size_t index, const void* data, size_t len) {
    jpg_chunking_t *j = (jpg_chunking_t *)arg;
    if (!index) {
        j->len = 0;
    }
    if (httpd_resp_send_chunk(j->req, (const char *)data, len) != ESP_OK) {
        return 0;
    }
    j->len += len;
    return len;
}

esp_err_t _image_handler(httpd_req_t *req) {
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;

  Serial.printf("Http: Image cnx started. Wifi RSSI %d\n", WiFi.RSSI());
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Http: esp_camera_fb_get failed");
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  int64_t fr_start = esp_timer_get_time();

  httpd_resp_set_type(req, "image/jpeg");
  httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

  size_t fb_len = 0;
  pixformat_t fb_format = fb->format;
  if (fb->format == PIXFORMAT_JPEG) {
    fb_len = fb->len;
    res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
  } else {
    jpg_chunking_t jchunk = {req, 0};
    res = frame2jpg_cb(fb, 80, _jpg_encode_stream, &jchunk) ? ESP_OK : ESP_FAIL;
    httpd_resp_send_chunk(req, NULL, 0);
    fb_len = jchunk.len;
  }
  esp_camera_fb_return(fb);

  int64_t fr_end = esp_timer_get_time();

  gStatImgCount++;
  if (gStatLastCaptureTs != 0) {
    gStatDeltaGrabMs = (uint32_t)((fr_start - gStatLastCaptureTs)/1000);
  }
  gStatLastCaptureTs = fr_start;
  gStatDeltaSendMs = (uint32_t)((fr_end - fr_start)/1000);
  Serial.printf("Http JPG: fmt=%d len=%u B grab=%u ms send=%u ms\n",
    fb_format,
    (uint32_t)(fb_len),
    gStatDeltaGrabMs,
    gStatDeltaSendMs);

  return res;
}

#define HTTP_BUF_LEN 1024
static char buf[HTTP_BUF_LEN];

esp_err_t _index_handler(httpd_req_t *req) {
  sensor_t * s = esp_camera_sensor_get();
  char *p = buf;

  Serial.printf("Http: Index cnx started. Wifi RSSI %d\n", WiFi.RSSI());

  p += sprintf(p, "<html><head><meta http-equiv=\"refresh\" content=\"1\"></head><body>\n");
  p += sprintf(p, "<p>Sensor: OV%02xxx\n", s->id.PID);
  p += sprintf(p, "<p>Image %u, grab=%u ms, send %u ms\n", gStatImgCount, gStatDeltaGrabMs, gStatDeltaSendMs);
  p += sprintf(p, "<p><img src='/img' />\n");
  p += sprintf(p, "</body></html>\n");
  *p++ = 0;

  size_t len = strlen(buf);
  if (len >= HTTP_BUF_LEN) {
    Serial.printf("ERROR BUFFER OVERFLOW. Increase HTML buf from %d to %d <<<<<\n", HTTP_BUF_LEN, len);
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, buf, len);
}

// In control.cpp
void _cam_control_init(httpd_handle_t streamHttpd, httpd_config_t &config);

void _http_start() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = HTTP_PORT;

  httpd_uri_t index_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = _index_handler,
    .user_ctx  = NULL
  };

  httpd_uri_t image_uri = {
    .uri       = "/img",
    .method    = HTTP_GET,
    .handler   = _image_handler,
    .user_ctx  = NULL
  };

  if (httpd_start(&gCameraHttpd, &config) == ESP_OK) {
    Serial.println("Http: Started");
    httpd_register_uri_handler(gCameraHttpd, &index_uri);
    httpd_register_uri_handler(gCameraHttpd, &image_uri);
    _cam_control_init(gCameraHttpd, config);
  } else {
    Serial.println("Http: Error starting");
  }
}

// ==== Wifi ====

bool gWifiConnected;

void _wifi_init() {
  WiFi.begin(gPrefWifiSsid.c_str(), gPrefWifiPass.c_str());
  gWifiConnected = false;
}

void _wifi_loop() {
  if (!gWifiConnected) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
    } else {
      gWifiConnected = true;
      Serial.printf("Wifi connected at http://%s port %d\n",
        WiFi.localIP().toString().c_str(),
        HTTP_PORT);
      _http_start();
    }
  } else {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.printf("Wifi status: %d\n", WiFi.status());
      // Q: do we need to try to reconnect or restart the http server?
    }
  }
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
  _camera_init();
  _wifi_init();
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  Serial.println("Loop");
  _wifi_loop();
  _blink();
}

