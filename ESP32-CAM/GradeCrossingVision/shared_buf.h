#ifndef __INC_SHARED_BUF_H
#define __INC_SHARED_BUF_H

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "common.h"


template <class P>
class SharedBufT {
public:
    SharedBufT(TaskHandle_t camera_task_handle, uint32_t request_bit) {
        mRequestBit = request_bit;
        mCameraTask = camera_task_handle;
        mDataQueue = xQueueCreate(
            1,            // uxQueueLength
            sizeof(P)     // uxItemSize
        );
        if (mDataQueue == NULL) {
            VERBOSE_PRINTF( ("FATAL: QueueCreate(%04x) == NULL\n", request_bit) );
        } else {
            VERBOSE_PRINTF( ("SharedBufT: QueueCreate(%04x) == %p, 1 x sizeof(%d)\n", request_bit, mDataQueue, sizeof(P)) );
        }
    }

    // --- Web handler API ---

    void request() {
        VERBOSE_PRINTF( ("SharedBufT::request val=%04d\n", mRequestBit) );
        xTaskNotify(mCameraTask, mRequestBit, eSetBits);
    }

    P receive(TickType_t ticks_to_wait) {
        P data = NULL;
        if (xQueueReceive(
                mDataQueue,  // xQueue,
                &data,       // pvBuffer,
                ticks_to_wait) == pdTRUE) {
            VERBOSE_PRINTF( ("SharedBufT::receive: %p\n", data) );
        }
        VERBOSE_PRINTF( ("SharedBufT::receive: %p\n", NULL) );
        return data;
    }

    // --- Camera task API ---

    bool queueIsEmpty() {
        VERBOSE_PRINTF( ("SharedBufT::queueIsEmpty: %d\n", (uxQueueSpacesAvailable(mDataQueue) > 0)) );
        return uxQueueSpacesAvailable(mDataQueue) > 0;
    }

    bool getAndResetRequest() {
        uint32_t value = 0;
        if (xTaskNotifyWait(
                0,            // ulBitsToClearOnEntry,
                mRequestBit,  // ulBitsToClearOnExit,
                &value,       // pulNotificationValue,
                0             // xTicksToWait
                ) == pdPASS) {
            VERBOSE_PRINTF( ("SharedBufT::getAndResetRequest: val=%04x\n", value) );
            return (value & mRequestBit) != 0;
        }

        VERBOSE_PRINTF( ("SharedBufT::getAndResetRequest: FALSE\n") );
        return false;
    }

    bool send(P data) {
        VERBOSE_PRINTF( ("SharedBufT::send: data=%p\n", data) );
        return xQueueSend(
            mDataQueue,  // xQueue,
            &data,       // pvItemToQueue,
            1            // xTicksToWait
        ) == pdTRUE;
    }


private:
    TaskHandle_t mCameraTask;
    QueueHandle_t mDataQueue;
    uint32_t mRequestBit;
};

#endif // __INC_SHARED_BUF_H
