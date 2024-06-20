#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <elapsedMillis.h>
#include <M5StickCPlus.h>

#include "Types.h"

namespace BuzzerTask
{
    TaskHandle_t taskHandle = NULL;
    const char *taskName = "BuzzerTask";

    // prototype
    InputPacket packet;

    elapsedMillis sinceBeep = 0;

    void task1(void *pvParameters)
    {
        Serial.printf("%s: Started\n", taskName);

        M5.Beep.tone(4000);
        while (sinceBeep < 500)
        {
            vTaskDelay(TICKS_5ms);
        }
        M5.Beep.mute();

        while (1)
        {
            vTaskDelay(TICKS_50ms);
        }
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
