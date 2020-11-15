#ifndef __INC_SHARED_BUF_H
#define __INC_SHARED_BUF_H

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include <HardwareSerial.h>

class SharedBuf {
public:
    SharedBuf(TaskHandle_t camera_task_handle, uint32_t request_bit);

    // Web handler API
    void request();
    void* receive(TickType_t ticks_to_wait);

    // Camera task API
    bool queueIsEmpty();
    bool getAndResetRequest();
    bool send(void *data);

private:
    TaskHandle_t mCameraTask;
    QueueHandle_t mDataQueue;
    uint32_t mRequestBit;
};


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
            Serial.printf("FATAL: QueueCreate(%04x) == NULL\n", request_bit);
        } else {
            Serial.printf("SharedBufT: QueueCreate(%04x) == %p\n", request_bit, mDataQueue);
        }
    }

    // --- Web handler API ---

    void request() {
        // Serial.printf("SharedBufT::request val=%04d\n", mRequestBit);
        xTaskNotify(mCameraTask, mRequestBit, eSetBits);
    }

    P receive(TickType_t ticks_to_wait) {
        P data = NULL;
        if (xQueueReceive(
                mDataQueue,  // xQueue,
                &data,       // pvBuffer,
                ticks_to_wait) == pdTRUE) {
            // Serial.printf("SharedBufT::receive: %p\n", msg.data);
        }
        // Serial.printf("SharedBufT::receive: %p\n", NULL);
        return data;
    }

    // --- Camera task API ---

    bool queueIsEmpty() {
        // Serial.printf("SharedBuf::queueIsEmpty: %d\n", (uxQueueSpacesAvailable(mDataQueue) > 0));
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
            // Serial.printf("SharedBufT::getAndResetRequest: val=%04x\n", value);
            return (value & mRequestBit) != 0;
        }

        // Serial.printf("SharedBufT::getAndResetRequest: FALSE\n");
        return false;
    }

    bool send(P data) {
        // Serial.printf("SharedBufT::send: data=%p\n", msg.data);
        return xQueueSend(
            mDataQueue,  // xQueue,
            data,        // pvItemToQueue,
            1            // xTicksToWait
        ) == pdTRUE;
    }


private:
    TaskHandle_t mCameraTask;
    QueueHandle_t mDataQueue;
    uint32_t mRequestBit;
};

#endif // __INC_SHARED_BUF_H
