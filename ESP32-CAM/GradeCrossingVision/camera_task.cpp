#include <esp_task_wdt.h>
#include <esp_camera.h>
#include <img_converters.h>

#include "common.h"
#include "cam_stats.h"
#include "shared_buf.h"
#include "cam_frame.h"
#include "cam_frame_pool.h"
#include "camera_task.h"

TaskHandle_t gCameraTask = NULL;
SharedBufT<CamFrameP> *gSharedCamImg = NULL;
CamStats gCamStats = {};    // <-- inits all fields to 0/null.


void cam_print_stats() {
    DEBUG_PRINTF( ("[Camera] Grb:%4d @ %2d ms > Cvt:%4d > Shr:%4d @ %2d [%d + %d] ms | Mem Int %ld B, Ext %ld B\n",
                  gCamStats.__frame_grab_count,
                  gCamStats.__frame_delta_ms,
                  gCamStats.__frame_convert_count,
                  gCamStats.__frame_share_count,
                  gCamStats.__process_delta_ms,
                  gCamStats.__process_malloc_ms,
                  gCamStats.__process_copy_ms,
                  heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
                  heap_caps_get_free_size(MALLOC_CAP_SPIRAM)
                  ) );
}

SharedBufT<CamFrameP> *cam_shared_img() { return gSharedCamImg; }


void _camera_task(void *taskParameters) {
    DEBUG_PRINTF( ("[Camera] Task running on Core %d\n", xPortGetCoreID()) );

    CamFramePool framePool(2);
    gSharedCamImg = new SharedBufT<CamFrameP>(gCameraTask, 1);

    esp_err_t err = esp_task_wdt_add(gCameraTask);
    if (err != ESP_OK) {
        ERROR_PRINTF( ("[Camera] Failed to add watchdog: err=%d\n", err) );
    } else {
        DEBUG_PRINTF( ("[Camera] Watchdog set %d\n", err) ); // -- timeouts = %d seconds\n", CONFIG_ESP_TASK_WDT_TIMEOUT_S) );
    }

    for (;;) {
        if (!is_ota_updating()) {
            // Note: all resources get deallocated when exiting this scope (RAII).
            CamFrameP f = framePool.getOrCreate(gCamStats);
            assert(f != NULL);
            if (f->grab()) {
                f->convert(); // SVGA --> 1.2sec
                if (f->share(gSharedCamImg)) {
                    framePool.relinquish(f);
                } else {
                    f->release_fb();
                    framePool.reuse(f);
                }
                gCamStats.__process_delta_ms = millis() - gCamStats.__frame_last_ms;
            }
        }

        // Reset watchdog for this task, and leave the idle task some time to process.
        err = esp_task_wdt_reset();
        rtDelay(1);
    }
}

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
    config.xclk_freq_hz = 20000000; // 20 MHz
    config.pixel_format = PIXFORMAT_JPEG;
    //config.pixel_format = PIXFORMAT_GRAYSCALE;
    //config.pixel_format = PIXFORMAT_RGB565; // fails
    //config.pixel_format = PIXFORMAT_YUV422; // fails
    //config.pixel_format = PIXFORMAT_RGB444; // not supported
    //config.pixel_format = PIXFORMAT_RGB555;
    //config.pixel_format = PIXFORMAT_RGB888;

    //init with high specs to pre-allocate larger buffers
    if (psramFound()) {
        DEBUG_PRINTF( ("[Camera] PSRAM found. Using SVGA size.\n") );
    } else {
        DEBUG_PRINTF( ("[Camera] ERROR PSRAM not found.\n") );
    }

    //config.frame_size = FRAMESIZE_SVGA;  // 800x600
    config.frame_size = FRAMESIZE_CIF;   // 400x296
    config.jpeg_quality = 10;
    config.fb_count = 2;

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ERROR_PRINTF( ("[Camera] init failed with error 0x%x\n", err) );
        return;
    }

    sensor_t *s = esp_camera_sensor_get();

    // Depending on how the sensor is mounted, this can be useful.
    // TBD: Make it a startup preference.
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);
}

void camera_task_init() {
    _esp_camera_init();

#if CONFIG_CAMERA_CORE0
    DEBUG_PRINTF( ("[Camera] ESP32-Camera task is pinned Core 0.\n") );
#elif CONFIG_CAMERA_CORE1
    DEBUG_PRINTF( ("[Camera] ESP32-Camera task is pinned Core 1.\n") );
#else
    DEBUG_PRINTF( ("[Camera] ESP32-Camera task has no core affinity.\n") );
#endif

    if (xTaskCreatePinnedToCore(
            _camera_task,  // pvTaskCode,
            "CameraTask",  // pcName (16 char max),
            4096,          // usStackDepth in bytes
            NULL,          // pvParameters,
            1,             // uxPriority, from 0 to configMAX_PRIORITIES
            &gCameraTask,  // pvCreatedTask
            PRO_CPU /*tskNO_AFFINITY*/) != pdPASS) {
        ERROR_PRINTF( ("[Camera] FATAL: Camera xTaskCreate failed.\n") );
    } else {
        DEBUG_PRINTF( ("[Camera]  Task created == %p\n", gCameraTask) );
    }
}

TaskHandle_t get_camera_task() {
    return gCameraTask;
}
