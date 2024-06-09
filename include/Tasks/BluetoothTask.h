#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace BluetoothTask
{
    void task1(void *pvParameters)
    {
        // (void)pvParameters;
        Serial.println("BluetoothTask: Started");

        Bluetooth::initialise();

        while (1)
        {
            Serial.println("BluetoothTask: Running");

            Bluetooth::PerformConnection();

            Bluetooth::PerformScan();

            vTaskDelay(500 / portTICK_PERIOD_MS); // Delay for 500 ms
        }
    }
}

// namespace BluetoothTask
// {
//     void taskRunner(void *pvParameters)
//     {
//         Serial.printf("Bluetooth Task started\n");

//         while (1)
//         {
//             Serial.println("Bluetooth task running");

//             vTaskDelay(100);
//         }
//     }
// };