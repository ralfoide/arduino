#ifndef __INC_WEB_TASK_H
#define __INC_WEB_TASK_H

#include <esp_http_server.h>
#include <String.h>

#include "shared_buf.h"
#include "cam_frame.h"

// In web_task.cpp
void wifi_init(const String &wifiSsid, const String &wifiPass);
void wifi_loop();
bool is_wifi_connected();

// Synchronous access to the shared camera task buffer
CamFrame *web_get_frame(int timeout_ms);
CamFrame *web_release_frame(CamFrame *frame);

// In web_config.cpp
void web_config_init(httpd_handle_t streamHttpd, httpd_config_t &config);

// From espressif app_httpd.cpp sample
typedef struct {
    httpd_req_t *req;
    size_t len;
} jpg_chunking_t;

#endif // __INC_WEB_TASK_H
