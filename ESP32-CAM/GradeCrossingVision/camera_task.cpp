#include <esp_camera.h>
#include <img_converters.h>
// #include <dl_lib_matrix3d.h>

#include <HardwareSerial.h>

#include "common.h"
#include "camera_task.h"
#include "shared_buf.h"

TaskHandle_t gCameraTask = NULL;
SharedBuf *gSharedCamImg = NULL;

long __frame_last_ms = 0;
int __frame_delta_ms = 0;
int __process_delta_ms = 0;
int __frame_grab_count = 0;
int __frame_share_count = 0;
int __frame_convert_count = 0;

void cam_print_stats() {
    Serial.printf("[Camera] Grb:%4d @ %2d ms > Cvt:%4d > Shr:%4d @ %2d ms | Mem Int %ld B, Ext %ld B\n",
                  __frame_grab_count,
                  __frame_delta_ms,
                  __frame_convert_count,
                  __frame_share_count,
                  __process_delta_ms,
                  heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
                  heap_caps_get_free_size(MALLOC_CAP_SPIRAM)
                  );
}

SharedBuf *cam_shared_img() { return gSharedCamImg; }

camera_fb_t *cam_dup_fb(camera_fb_t *src) {
    // Serial.printf("[cam] cam_dup_fb fb %p", src);
    assert(src != NULL);
    // Serial.printf("... len=%d, buf=%p\n", src, src->len, src->buf);
    camera_fb_t *dst = (camera_fb_t *) malloc(sizeof(camera_fb_t));
    assert(dst != NULL);

    memcpy(dst, src, sizeof(camera_fb_t));

    dst->buf = NULL;
    size_t len = dst->len;
    if (len > 0 && src->buf != NULL) {
        dst->buf = (uint8_t *) heap_caps_malloc(len, MALLOC_CAP_SPIRAM);
        assert(dst->buf != NULL);
        memcpy(dst->buf, src->buf, len);
    }

    return dst;
}

void cam_free_fb(camera_fb_t *fb) {
    if (fb == NULL) return;
    if (fb->buf) {
        free(fb->buf);
        fb->buf = NULL;
    }
    free(fb);
}

class CamFrame {
private:
    camera_fb_t *fb;
    uint8_t *img_rgb;

public:
    CamFrame() : 
        fb(NULL),
        img_rgb(NULL) {
    }

    ~CamFrame() {
        // Important. We must not leave this without releasing the frame buffer.
        release_fb();
        release_img();
    }

    bool grab() {
        fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("[Camera] esp_camera_fb_get failed");
        }

        __frame_grab_count++;

        long ms = millis();
        if (__frame_last_ms != 0) {
            __frame_delta_ms = ms - __frame_last_ms;
        }
        __frame_last_ms = ms;

        return fb != NULL;
    }

    void release_fb() {
        if (fb) {
            esp_camera_fb_return(fb);
            fb = NULL;
        }
    }

    void release_img() {
        if (img_rgb) {
            free(img_rgb);
            img_rgb = NULL;
        }
    }

    // Unpacks the JPEG data into an RGB buffer.
    // Note: JPEG frame buffer is not freed yet.
    void convert() {
        if (fb == NULL) return;

        size_t len = fb->width * fb->height * 3;
        img_rgb = (uint8_t *) heap_caps_malloc(len, MALLOC_CAP_SPIRAM);
        if (!img_rgb) {
            Serial.printf("[Camera] malloc %ld bytes failed\n", len);
            heap_caps_print_heap_info(MALLOC_CAP_INTERNAL);
            heap_caps_print_heap_info(MALLOC_CAP_SPIRAM);
        }
        assert(img_rgb != NULL);

        if (!fmt2rgb888(fb->buf, fb->len, fb->format, img_rgb)) {
            Serial.println("[Camera] fmt2rgb888 failed");
            return;
        }

        // TBD: We don't currently use the unpacked RGB buffer for anything.
        __frame_convert_count++;
    }

    // Shared the JPEG frame buffer.
    void share() {
        if (fb == NULL || gSharedCamImg == NULL) return;

        if (gSharedCamImg->queueIsEmpty() && gSharedCamImg->getAndResetRequest()) {
            camera_fb_t *fb2 = cam_dup_fb(fb);
            Serial.printf("[cam] SEND    fb %p --> %dx%d, fmt=%d, len=%d, buf=%p\n", fb2, fb2->width, fb2->height, fb2->format, fb2->len, fb2->buf);
            if (gSharedCamImg->send(fb2)) {
                __frame_share_count++;
            } else {
                Serial.println("[Camera] gSharedCamImg.send failed");
                cam_free_fb(fb2);
            }
        }
    }
};

void _camera_task(void *taskParameters) {
    Serial.printf("[Camera] Task running on Core %d\n", xPortGetCoreID());

    gSharedCamImg = new SharedBuf(gCameraTask, 1);

    for (;;) {
        if (!is_ota_updating()) {
            // Note: all resources get deallocated when exiting this scope (RAII).
            CamFrame f = CamFrame();
            if (f.grab()) {
                //--f.convert(); --> 1.2sec
                f.share();
                __process_delta_ms = millis() - __frame_last_ms;
            }
        }
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
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    //init with high specs to pre-allocate larger buffers
    if (psramFound()) {
        Serial.println("[Camera] PSRAM found. Using SVGA size.");
    } else {
        Serial.println("[Camera] ERROR PSRAM not found.");
    }

    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("[Camera] init failed with error 0x%x\n", err);
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
    Serial.println("[Camera] ESP32-Camera task is pinned Core 0.");
#elif CONFIG_CAMERA_CORE1
    Serial.println("[Camera] ESP32-Camera task is pinned Core 1.");
#else
    Serial.println("[Camera] ESP32-Camera task has no core affinity.");
#endif

    if (xTaskCreatePinnedToCore(
            _camera_task,  // pvTaskCode,
            "CameraTask",  // pcName (16 char max),
            4096,          // usStackDepth in bytes
            NULL,          // pvParameters,
            0,             // uxPriority, from 0 to configMAX_PRIORITIES
            &gCameraTask,  // pvCreatedTask
            PRO_CPU /*tskNO_AFFINITY*/) != pdPASS) {
        Serial.println("[Camera] FATAL: Camera xTaskCreate failed.");
    } else {
        Serial.printf("[Camera]  Task created == %p\n", gCameraTask);
    }
}

TaskHandle_t get_camera_task() {
    return gCameraTask;
}
