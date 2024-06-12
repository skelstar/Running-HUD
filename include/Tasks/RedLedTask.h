#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <elapsedMillis.h>

namespace RedLedTask
{
    TaskHandle_t taskHandle = NULL;
    const char *taskName = "RedLedTask";

    elapsedMillis sinceFlashedLed = 0;
    bool m5ledState = false;

    void task1(void *pvParameters)
    {
        Serial.printf("%s: Started\n", taskName);

        pinMode(M5_LED_PIN, OUTPUT);

        while (1)
        {
            uint16_t blinkMs = m5ledState == 0 ? 100 : 2000;
            if (sinceFlashedLed > blinkMs)
            {
                sinceFlashedLed = 0;
                m5ledState = !m5ledState;
                digitalWrite(M5_LED_PIN, m5ledState);
            }

            vTaskDelay(TICKS_50ms);
        }
    }
}
