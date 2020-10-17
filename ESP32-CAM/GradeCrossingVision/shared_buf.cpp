#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <HardwareSerial.h>

#include "shared_buf.h"

struct QMsg {
    void* data;
};

SharedBuf::SharedBuf(TaskHandle_t camera_task_handle, uint32_t request_bit) {
    mRequestBit = request_bit;
    mCameraTask = camera_task_handle;
    mDataQueue = xQueueCreate(
        1,            // uxQueueLength
        sizeof(QMsg)  // uxItemSize
    );
    if (mDataQueue == NULL) {
        Serial.printf("FATAL: QueueCreate(%04x) == NULL\n", request_bit);
    } else {
        Serial.printf("SharedBuf: QueueCreate(%04x) == %p\n", request_bit, mDataQueue);
    }
}

// --- Web handler API ---

void SharedBuf::request() {
    xTaskNotify(mCameraTask, mRequestBit, eSetBits);
}

void* SharedBuf::receive(TickType_t ticks_to_wait) {
    QMsg msg;
    if (xQueueReceive(
            mDataQueue,  // xQueue,
            &msg,        // pvBuffer,
            ticks_to_wait) == pdTRUE) {
        return msg.data;
    }
    return NULL;
}

// --- Camera task API ---

bool SharedBuf::queueIsEmpty() {
    return uxQueueSpacesAvailable(mDataQueue) > 0;
}

bool SharedBuf::getAndResetRequest() {
    uint32_t value = 0;
    if (xTaskNotifyWait(
            0,            // ulBitsToClearOnEntry,
            mRequestBit,  // ulBitsToClearOnExit,
            &value,       // pulNotificationValue,
            0             // xTicksToWait
            ) == pdPASS) {
        return (value & mRequestBit) != 0;
    }

    return false;
}

void SharedBuf::send(void* data) {
    QMsg msg;
    msg.data = data;

    xQueueSend(
        mDataQueue,  // xQueue,
        &msg,        // pvItemToQueue,
        0            // xTicksToWait
    );
}
