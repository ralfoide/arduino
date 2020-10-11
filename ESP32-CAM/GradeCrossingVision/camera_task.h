#ifndef __INC_CAMERA_TASK_H
#define __INC_CAMERA_TASK_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Set camera model
#define CAMERA_MODEL_AI_THINKER   // for ESP32-CAM module
#include "camera_pins.h"

void camera_task_init();
TaskHandle_t get_camera_task();


#endif // __INC_CAMERA_TASK_H
