#include <esp_http_server.h>
#include <esp_timer.h>
#include <img_converters.h>
#include <fb_gfx.h>
#include <WiFi.h>
#include <String.h>

#include "common.h"
#include "web_task.h"


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

  Serial.printf("Http: Index cnx started. Wifi RSSI %d, Core %d\n", WiFi.RSSI(), xPortGetCoreID());

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
    web_config_init(gCameraHttpd, config);
  } else {
    Serial.println("Http: Error starting");
  }
}

// ==== Wifi ====

bool gWifiConnected;

void wifi_init(const String &wifiSsid, const String &wifiPass) {
  WiFi.begin(wifiSsid.c_str(), wifiPass.c_str());
  gWifiConnected = false;
}

void wifi_loop() {
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


