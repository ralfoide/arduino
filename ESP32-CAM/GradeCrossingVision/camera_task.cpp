#include <esp_camera.h>
#include <img_converters.h>

#include <HardwareSerial.h>

#include "common.h"
#include "camera_task.h"
#include "shared_buf.h"

TaskHandle_t gCameraTask = NULL;
SharedBuf *gSharedCamImg = NULL;

void _esp_camera_init() {
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
    } else {
        Serial.println("Camera: ERROR PSRAM not found.");
    }

    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera: init failed with error 0x%x\n", err);
        return;
    }

    sensor_t *s = esp_camera_sensor_get();

    // Depending on how the sensor is mounted, this can be useful.
    // TBD: Make it a startup preference.
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);
}

void _camera_task(void *taskParameters) {
    Serial.printf("Camera Task running on Core %d\n", xPortGetCoreID());
    
    gSharedCamImg = new SharedBuf(gCameraTask, 1);

    for (;;) {
        int64_t fr_start = esp_timer_get_time();
        delay(10000);    // TODO change
       Serial.printf("Camera Task Loop running on Core %d\n", xPortGetCoreID());
    }
}

void camera_task_init() {
    _esp_camera_init();

    #if CONFIG_CAMERA_CORE0
        Serial.println("ESP32-Camera task is pinned Core 0.");
    #elif CONFIG_CAMERA_CORE1
        Serial.println("ESP32-Camera task is pinned Core 1.");
    #else
        Serial.println("ESP32-Camera task has no core affinity.");
    #endif

    if (xTaskCreatePinnedToCore(
            _camera_task,           // pvTaskCode,
            "CameraTask",           // pcName (16 char max),
            4096,                   // usStackDepth in bytes
            NULL,                   // pvParameters,
            0,                      // uxPriority, from 0 to configMAX_PRIORITIES
            &gCameraTask,           // pvCreatedTask
            PRO_CPU /*tskNO_AFFINITY*/) != pdPASS) {
        Serial.println("FATAL: Camera xTaskCreate failed.");
    } else {
        Serial.printf("Camera: Task created == %p\n", gCameraTask);
    }
}

TaskHandle_t get_camera_task() {
    return gCameraTask;
}
