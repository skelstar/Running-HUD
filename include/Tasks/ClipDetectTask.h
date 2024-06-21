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
    void sendPacket(InputPacket *packet);
    bool clipDetected();

    InputPacket packet;
    elapsedMillis sinceCheckedClip = 0;

    void task1(void *pvParameters)
    {
        Serial.printf("%s: Started\n", taskName);

        pinMode(CLIP_DETECT_PIN, INPUT_PULLUP);

        while (1)
        {
            bool detected = clipDetected();

            packet.id++;
            packet.input = InputOption::CLIP_DETECT;
            packet.event = detected ? ButtonEvent::DETECTED : ButtonEvent::NOT_DETECTED;

            sendPacket(&packet);

            vTaskDelay(TICKS_500ms);
        }
    }

    void sendPacket(InputPacket *packet)
    {
        InputPacket *data;
        data = packet;
        xQueueSend(xInputsQueue, (void *)&data, TICKS_10ms);
    }

    bool clipDetected()
    {
        return digitalRead(CLIP_DETECT_PIN) == 0;
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
