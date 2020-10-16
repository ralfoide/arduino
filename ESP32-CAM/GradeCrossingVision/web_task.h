#ifndef __INC_WEB_TASK_H
#define __INC_WEB_TASK_H

#include <esp_http_server.h>
#include <String.h>

// In web_task.cpp
void wifi_init(const String &wifiSsid, const String &wifiPass);
void wifi_loop();

// In web_control.cpp
void web_config_init(httpd_handle_t streamHttpd, httpd_config_t &config);

// From espressif app_httpd.cpp sample
typedef struct {
        httpd_req_t *req;
        size_t len;
} jpg_chunking_t;

#endif // __INC_WEB_TASK_H
