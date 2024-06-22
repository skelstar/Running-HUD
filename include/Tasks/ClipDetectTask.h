#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <elapsedMillis.h>

#include "Types.h"

namespace ClipDetectTask
{
    TaskHandle_t taskHandle = NULL;
    const char *taskName = "ClipDetectTask";

    // prototype
    void sendDetected(bool detected);

#define G25 25
#define G36 36

    ClipDetectPacket packet;
    elapsedMillis sinceCheckedClip = 0;

    void task1(void *pvParameters)
    {
        Serial.printf("%s: Started\n", taskName);

        pinMode(G36, GPIO_FLOATING);
        pinMode(G25, INPUT_PULLDOWN);

        while (1)
        {
            bool detected = digitalRead(G25) == 1;
            sendDetected(detected);

            vTaskDelay(TICKS_1s);
        }
    }

    void sendDetected(bool detected)
    {
        packet.id++;
        packet.status = detected ? CLIP_DETECTED : CLIP_NOT_DETECTED;

        ClipDetectPacket *data;
        data = &packet;
        xQueueSendToFront(xClipDetectQueue, (void *)&data, TICKS_10ms);
        // Serial.printf("sending... CLIP: %s id: %d \n", detected ? "DETECTED" : "NOT DETECTED!", packet.id);
    }

    void createTask(int stackDepth)
    {
        xTaskCreatePinnedToCore(
            task1,
            taskName,
            /*stack depth*/ stackDepth,
            /*params*/ NULL,
            /*priority*/ 1,
            &taskHandle,
            /*core*/ 1);
    }
}
