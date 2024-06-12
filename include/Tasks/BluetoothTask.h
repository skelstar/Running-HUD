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

        Bluetooth::initialise();

        while (1)
        {
            Bluetooth::PerformConnection();

            Bluetooth::PerformScan();

            vTaskDelay(TICKS_100ms);
        }
    }
}
