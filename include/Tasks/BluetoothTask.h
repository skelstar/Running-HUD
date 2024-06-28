#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "Bluetooth.h"

namespace BluetoothTask
{
    TaskHandle_t taskHandle = NULL;
    const char *taskName = "BluetoothTask";

    void task1(void *pvParameters)
    {
        Serial.printf("%s: Started\n", taskName);

        vTaskDelay(THREE_SECONDS);

        Bluetooth::initialise();

        while (1)
        {
            Bluetooth::PerformConnection();

            Bluetooth::startScanning();

            vTaskDelay(TICKS_100ms);
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