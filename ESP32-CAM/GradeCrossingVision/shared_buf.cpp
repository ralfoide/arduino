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
    // Serial.printf("SharedBuf::request val=%04d\n", mRequestBit);
    xTaskNotify(mCameraTask, mRequestBit, eSetBits);
}

void* SharedBuf::receive(TickType_t ticks_to_wait) {
    QMsg msg;
    if (xQueueReceive(
            mDataQueue,  // xQueue,
            &msg,        // pvBuffer,
            ticks_to_wait) == pdTRUE) {
        Serial.printf("SharedBuf::receive: %p\n", msg.data);
        return msg.data;
    }
    // Serial.printf("SharedBuf::receive: %p\n", NULL);
    return NULL;
}

// --- Camera task API ---

bool SharedBuf::queueIsEmpty() {
    // Serial.printf("SharedBuf::queueIsEmpty: %d\n", (uxQueueSpacesAvailable(mDataQueue) > 0));
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
        // Serial.printf("SharedBuf::getAndResetRequest: val=%04x\n", value);
        return (value & mRequestBit) != 0;
    }

        // Serial.printf("SharedBuf::getAndResetRequest: FALSE\n");
    return false;
}

bool SharedBuf::send(void* data) {
    QMsg msg;
    msg.data = data;

    Serial.printf("SharedBuf::send: data=%p\n", msg.data);
    return xQueueSend(
        mDataQueue,  // xQueue,
        &msg,        // pvItemToQueue,
        1            // xTicksToWait
    ) == pdTRUE;
}
