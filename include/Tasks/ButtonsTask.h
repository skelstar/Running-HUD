#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "ButtonAcc.h"
#include "ButtonMain.h"
#include "Types.h"

namespace ButtonsTask
{
    TaskHandle_t taskHandle = NULL;
    const char *taskName = "ButtonsTask";

    void task1(void *pvParameters)
    {
        Serial.println("ButtonsTask: Started");

        ButtonMain::initialise();
        ButtonAcc::initialise();

        while (1)
        {
            ButtonMain::button.loop();
            ButtonAcc::button.loop();

            vTaskDelay(TICKS_100ms);
        }
    }
}