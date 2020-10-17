#ifndef __INC_COMMON_H
#define __INC_COMMON_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define rtDelay(ms) { vTaskDelay((ms) / portTICK_PERIOD_MS); } // delay in mS

// CPU affinity for ESP32
#define PRO_CPU 0
#define APP_CPU 1

// In main INO
bool is_ota_updating();

#endif // __INC_COMMON_H
