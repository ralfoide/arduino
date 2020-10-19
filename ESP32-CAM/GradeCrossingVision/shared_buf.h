#ifndef __INC_SHARED_BUF_H
#define __INC_SHARED_BUF_H

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

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

#endif // __INC_SHARED_BUF_H
