#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <elapsedMillis.h>

namespace RedLedTask
{
    TaskHandle_t taskHandle = NULL;
    const char *taskName = "RedLedTask";

    void handlePacket(Bluetooth::Packet *packet);

    elapsedMillis sinceFlashedLed = 0, sinceFlashRedLedCommand = 0;
    bool redLedState = false;
    unsigned long commandId = -1;

    void task1(void *pvParameters)
    {
        Serial.printf("%s: Started\n", taskName);

        pinMode(RED_LED_PIN, OUTPUT);
        digitalWrite(RED_LED_PIN, HIGH); // off

        while (1)
        {
            CommandPacket *commandPacket = nullptr;

            if (xQueuePeek(xCommandQueue, &(commandPacket), TICKS_50ms) == pdTRUE)
            {
                if (commandPacket->id != commandId)
                {
                    commandId = commandPacket->id;

                    if (commandPacket->command == COMMAND_FLASH_RED_LED)
                        sinceFlashRedLedCommand = 0;
                }
            }

            uint16_t blinkMs = redLedState == 0 ? 50 : THREE_SECONDS;
            if (sinceFlashedLed > blinkMs &&
                sinceFlashRedLedCommand < ONE_SECONDS)
            {
                sinceFlashedLed = 0;
                redLedState = !redLedState;
            }
            else
            {
                redLedState = 1; // OFF
            }
            digitalWrite(RED_LED_PIN, redLedState);

            vTaskDelay(TICKS_10ms);
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
