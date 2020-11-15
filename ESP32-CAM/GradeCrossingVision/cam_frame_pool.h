#ifndef __INC_CAM_FRAME_POOL_H
#define __INC_CAM_FRAME_POOL_H


#include <HardwareSerial.h>

#include "cam_stats.h"
#include "cam_frame.h"

class CamFramePool {
private:
    int _size;
    int _inUse;
    CamFrameP *_frames;

public:
    CamFramePool(int size) :
        _size(size),
        _inUse(0) {
        _frames = new CamFrameP[size];
    }

    ~CamFramePool() {
        delete[] _frames;
    }

    CamFrameP getOrCreate(CamStats &stats) {
        for (int i = 0; i < _size; i++) {
            if (!is_bit_set(_inUse, i)) {
                _inUse = set_bit(_inUse, i);
                CamFrameP frame = _frames[i];
                if (frame == NULL) {
                    frame = new CamFrame(stats);
                    _frames[i] = frame;
                }
                return frame;
            }
        }
        return NULL;
    }

    void reuse(CamFrameP frame) {
        for (int i = 0; i < _size; i++) {
            if (_frames[i] == frame) {
                _inUse = clear_bit(_inUse, i);
                return;
            }
        }
    }

    void relinquish(CamFrameP frame) {
        for (int i = 0; i < _size; i++) {
            if (_frames[i] == frame) {
                _inUse = clear_bit(_inUse, i);
                _frames[i] = NULL;
                return;
            }
        }
    }

private:
    bool is_bit_set(int val, int index) {
        return (val & (1 << index)) != 0;
    }

    int clear_bit(int val, int index) {
        return val & ~(1 << index);
    }

    int set_bit(int val, int index) {
        return val | (1 << index);
    }
};

#endif // __INC_CAM_FRAME_POOL_H
