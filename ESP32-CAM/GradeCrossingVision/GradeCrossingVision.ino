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
    Serial.println("Camera: PSRAM found, UXGA size");
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    Serial.println("Camera: SVGA size");
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  // Not used on ESP32-CAM
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera: init failed with error 0x%x\n", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    Serial.println("Camera: OV3660 PID");
    s->set_vflip(s, 1);       //flip it back
    s->set_brightness(s, 1);  //up the blightness just a bit
    s->set_saturation(s, -2); //lower the saturation
  }

  // [RM] drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE)
  // Not used on ESP32-CAM
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif
}

// ==== HTTP Server ====

#define PART_BOUNDARY "123456789000000000000987654321"
#define HTTP_PORT 80
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t gStreamHttpd = NULL;

esp_err_t _stream_handler(httpd_req_t *req) {
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  size_t jpg_buf_len = 0;
  uint8_t *jpg_buf = NULL;
  char *part_buf[64];

  Serial.println("Http: Stream cnx started");
  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK) {
    Serial.printf("Http: httpd_resp_set_type res 0x%x\n", res);
    return res;
  }

  while (true) {
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Http: esp_camera_fb_get fail\n");
      res = ESP_FAIL;
    } else {
      if (fb->width > 400) {
        if (fb->format != PIXFORMAT_JPEG) {
          bool jpeg_converted = frame2jpg(fb, 80, &jpg_buf, &jpg_buf_len);
          esp_camera_fb_return(fb);
          fb = NULL;
          if (!jpeg_converted) {
            Serial.println("Http: frame2jpg JPEG compression failed");
            res = ESP_FAIL;
          }
        } else {
          jpg_buf_len = fb->len;
          jpg_buf = fb->buf;
        }
      }
    }
    if (res == ESP_OK) {
      size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, (const char *)jpg_buf, jpg_buf_len);
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if (fb) {
      esp_camera_fb_return(fb);
      fb = NULL;
      jpg_buf = NULL;
    } else if (jpg_buf) {
      free(jpg_buf);
      jpg_buf = NULL;
    }
    if (res != ESP_OK) {
      Serial.printf("Http: httpd_resp_send_chunk res 0x%x\n", res);
      break;
    }
    //Serial.printf("MJPG: %uB\n",(uint32_t)(_jpg_buf_len));
  }

  Serial.printf("Http: Stream cnx end res 0x%x\n", res);
  return res;
}

// From espressif app_httpd.cpp sample
typedef struct {
        httpd_req_t *req;
        size_t len;
} jpg_chunking_t;

size_t _jpg_encode_stream(void * arg, size_t index, const void* data, size_t len){
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
  int64_t fr_start = esp_timer_get_time();

  Serial.println("Http: Image cnx started");
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Http: esp_camera_fb_get failed");
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

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
  Serial.printf("Http JPG: fmt=%d len=%uB time=%ums\n",
    fb_format,
    (uint32_t)(fb_len),
    (uint32_t)((fr_end - fr_start)/1000));
  return res;
}

void _http_start() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = HTTP_PORT;

  httpd_uri_t index_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = _image_handler,
    .user_ctx  = NULL
  };
  
  if (httpd_start(&gStreamHttpd, &config) == ESP_OK) {
    Serial.println("Http: Started");
    httpd_register_uri_handler(gStreamHttpd, &index_uri);
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

