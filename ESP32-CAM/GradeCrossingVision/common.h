#ifndef __INC_COMMON_H
#define __INC_COMMON_H

// CPU affinity for ESP32
#define PRO_CPU 0
#define APP_CPU 1

// From espressif app_httpd.cpp sample
typedef struct {
        httpd_req_t *req;
        size_t len;
} jpg_chunking_t;

#endif // __INC_COMMON_H
