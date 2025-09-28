#ifndef __INC_CAM_FRAME_H
#define __INC_CAM_FRAME_H

#include <esp_camera.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "common.h"
#include "cam_stats.h"
#include "shared_buf.h"

class CamFrame;
typedef CamFrame * CamFrameP;

class CamFrame {
private:
    camera_fb_t *_fb;
    uint8_t *_img_rgb;
    CamStats &_stats;

public:
    CamFrame(CamStats &stats) : 
        _fb(NULL),
        _img_rgb(NULL),
        _stats(stats) {
        VERBOSE_PRINTF( ("CamFrame::new %p\n", this) );
    }

    ~CamFrame() {
        // Important. We must not leave this without releasing the frame buffer.
        VERBOSE_PRINTF( ("CamFrame::release %p\n", this) );
        release_fb();
        release_img();
    }

    camera_fb_t *fb() {
        return _fb;
    }

    bool grab() {
        VERBOSE_PRINTF( ("CamFrame::grab %p\n", this) );
        assert(_fb == NULL); // client should have called release_fb() before.

        _fb = esp_camera_fb_get();
        if (!_fb) {
            ERROR_PRINTF( ("CamFrame::grab esp_camera_fb_get failed\n") );
        }

        _stats.__frame_grab_count++;

        long ms = millis();
        if (_stats.__frame_last_ms != 0) {
            _stats.__frame_delta_ms = ms - _stats.__frame_last_ms;
        }
        _stats.__frame_last_ms = ms;

        return _fb != NULL;
    }

    void release_fb() {
        if (_fb) {
            esp_camera_fb_return(_fb);
            _fb = NULL;
        }
    }

    void release_img() {
        if (_img_rgb) {
            free(_img_rgb);
            _img_rgb = NULL;
        }
    }

    void convert() {
        VERBOSE_PRINTF( ("CamFrame::convert %p\n", this) );
        if (_fb == NULL) return;

        // TODO only reallocate if needed
        release_img();

        if (_fb->format == PIXFORMAT_JPEG) {
            // Unpacks the JPEG data into an RGB buffer.
            // Note: JPEG frame buffer is freed by ~CamFrame (end of scaope in camera task loop).

            long ms1 = millis();
            size_t len = _fb->width * _fb->height * 3;
            _img_rgb = (uint8_t *) heap_caps_malloc(len, MALLOC_CAP_SPIRAM);
            long ms2 = millis();
            _stats.__process_malloc_ms = ms2 - ms1;
            if (!_img_rgb) {
                ERROR_PRINTF( ("[Camera] malloc %ld bytes failed\n", len) );
                heap_caps_print_heap_info(MALLOC_CAP_INTERNAL);
                heap_caps_print_heap_info(MALLOC_CAP_SPIRAM);
            }
            assert(_img_rgb != NULL);

            if (!fmt2rgb888(_fb->buf, _fb->len, _fb->format, _img_rgb)) {
                ERROR_PRINTF( ("[Camera] fmt2rgb888 failed\n") );
                return;
            }
            _stats.__process_copy_ms = millis() - ms2;
        }

        // TBD: We don't currently use the unpacked RGB buffer for anything.
        _stats.__frame_convert_count++;
    }

    // Share the JPEG frame buffer. Returns true if it is being shared successfully.
    bool share(SharedBufT<CamFrameP> *sharedCamFrame) {
        VERBOSE_PRINTF( ("CamFrame::share %p to SharedBufT %p\n", this, sharedCamFrame) );
        if (sharedCamFrame == NULL) return false;

        if (sharedCamFrame->queueIsEmpty() && sharedCamFrame->getAndResetRequest()) {
            if (sharedCamFrame->send(this)) {
                _stats.__frame_share_count++;
                return true;
            } else {
                ERROR_PRINTF( ("CamFrame::share sharedCamFrame.send failed\n") );
            }
        }

        return false;
    }
};

#endif // __INC_CAM_FRAME_H
