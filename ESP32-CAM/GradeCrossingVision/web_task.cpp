#include <esp_http_server.h>
#include <esp_timer.h>
#include <img_converters.h>
#include <fb_gfx.h>
#include <WiFi.h>
#include <String.h>

#include "common.h"
#include "web_task.h"
#include "camera_task.h"
#include "shared_buf.h"
#include "cam_frame.h"

// ==== HTTP Server ====

#define HTTP_PORT 80
httpd_handle_t gWebHttp = NULL;

long gStatImgCount = 0;
long gStatLastCaptureMs = 0;
long gStatDeltaGrabMs = 0;
long gStatDeltaSendMs = 0;

CamFrameP web_get_frame(int timeout_ms) {
    SharedBufT<CamFrameP> *sharedBufImg = cam_shared_img();
    if (sharedBufImg == NULL) return NULL;

    sharedBufImg->request();
    CamFrameP data = sharedBufImg->receive(MS_TO_TICKS(timeout_ms));
    // Serial.printf("[web] web_get_fb data = %p\n", data);
    return data;
}

CamFrameP web_release_frame(CamFrameP frame) {
    if (frame != NULL) {
        delete frame;
    }
    return NULL;
}

size_t _jpg_encode_stream(void *arg, size_t index, const void *data, size_t len) {
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
    CamFrameP frame = NULL;
    esp_err_t res = ESP_OK;

    Serial.printf("[HTTP] Image cnx started. Wifi RSSI %d\n", WiFi.RSSI());
    frame = web_get_frame(250 /*ms*/);
    if (!frame || !frame->fb()) {
        frame = web_release_frame(frame);
        Serial.println("[HTTP] web_get_fb failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    long fr_start_ms = millis();

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    size_t fb_len = 0;
    pixformat_t fb_format = frame->fb()->format;
    if (frame->fb()->format == PIXFORMAT_JPEG) {
        fb_len = frame->fb()->len;
        res = httpd_resp_send(req, (const char *)frame->fb()->buf, fb_len);
    } else {
        jpg_chunking_t jchunk = {req, 0};
        res = frame2jpg_cb(frame->fb(), 80, _jpg_encode_stream, &jchunk) ? ESP_OK : ESP_FAIL;
        httpd_resp_send_chunk(req, NULL, 0);  // this frees the buffers used by _jpg_encode_stream
        fb_len = jchunk.len;
    }
    frame = web_release_frame(frame);

    long fr_end_ms = millis();

    gStatImgCount++;
    if (gStatLastCaptureMs != 0) {
        gStatDeltaGrabMs = fr_start_ms - gStatLastCaptureMs;
    }
    gStatLastCaptureMs = fr_start_ms;
    gStatDeltaSendMs = fr_end_ms - fr_start_ms;
    Serial.printf("[HTTP] JPG: fmt=%d len=%d B grab=%d ms send=%d ms\n",
                  fb_format,
                  fb_len,
                  gStatDeltaGrabMs,
                  gStatDeltaSendMs);

    return res;
}

#define HTTP_BUF_LEN 1024
static char buf[HTTP_BUF_LEN];

esp_err_t _index_handler(httpd_req_t *req) {
    sensor_t *s = esp_camera_sensor_get();
    char *p = buf;

    Serial.printf("[HTTP] Index cnx started. Wifi RSSI %d, Core %d\n", WiFi.RSSI(), xPortGetCoreID());

    p += sprintf(p, "<html><head><meta http-equiv=\"refresh\" content=\"1\"></head><body>\n");
    p += sprintf(p, "<p>Sensor: OV%02xxx\n", s->id.PID);
    p += sprintf(p, "<p>Image %u, grab=%d ms, send %d ms\n", gStatImgCount, gStatDeltaGrabMs, gStatDeltaSendMs);
    p += sprintf(p, "<p><img src='/img' />\n");
    p += sprintf(p, "</body></html>\n");
    *p++ = 0;

    size_t len = strlen(buf);
    if (len >= HTTP_BUF_LEN) {
        Serial.printf("[HTTP] ERROR BUFFER OVERFLOW. Increase HTML buf from %d to %d <<<<<\n", HTTP_BUF_LEN, len);
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
        .uri = "/",
        .method = HTTP_GET,
        .handler = _index_handler,
        .user_ctx = NULL};

    httpd_uri_t image_uri = {
        .uri = "/img",
        .method = HTTP_GET,
        .handler = _image_handler,
        .user_ctx = NULL};

    if (httpd_start(&gWebHttp, &config) == ESP_OK) {
        Serial.printf("[HTTP] Started on port %d, httpd %p\n", config.server_port, gWebHttp);
        httpd_register_uri_handler(gWebHttp, &index_uri);
        httpd_register_uri_handler(gWebHttp, &image_uri);
        web_config_init(gWebHttp, config);
    } else {
        Serial.println("[HTTP] Error starting");
    }
}

// ==== Wifi ====

bool gWifiConnected;

bool is_wifi_connected() {
    return gWifiConnected;
}

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
            Serial.printf("\nWifi connected at http://%s port %d\n",
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
