#ifndef __INC_CAMERA_TASK_H
#define __INC_CAMERA_TASK_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "shared_buf.h"
#include "cam_frame.h"

// Set camera model
#define CAMERA_MODEL_AI_THINKER   // for ESP32-CAM module
#include "camera_pins.h"

void camera_task_init();
void cam_free_fb(camera_fb_t *fb);

TaskHandle_t get_camera_task();
SharedBufT<CamFrameP> *cam_shared_img();
void cam_print_stats();

#endif // __INC_CAMERA_TASK_H
