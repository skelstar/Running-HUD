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
    void handleCommandPacket(CommandPacket *packet);
    bool isNewPacket(unsigned long newId);

    elapsedMillis sinceBeep = 0;
    unsigned long commandPacketId = -1;
    CommandPacket *commandPacket1 = nullptr;

    void task1(void *pvParameters)
    {
        Serial.printf("%s: Started\n", taskName);

        sinceBeep = 0;
        M5.Beep.tone(4000);
        while (sinceBeep < 500)
        {
            vTaskDelay(TICKS_5ms);
        }
        M5.Beep.mute();

        while (1)
        {

            CommandPacket *commandPacket = nullptr;
            if (xQueuePeek(xCommandQueue, &(commandPacket), TICKS_50ms) == pdTRUE &&
                isNewPacket(commandPacket->id))
            {
                handleCommandPacket(commandPacket);
            }

            vTaskDelay(TICKS_5ms);
        }
    }

    bool isNewPacket(unsigned long newId)
    {
        bool newPacket = newId != commandPacketId;
        if (newId - commandPacketId > 1)
        {
            Serial.printf("Command Rx id: %lu (commandPacketId: %lu) \n",
                          newId, commandPacketId);
        }
        commandPacketId = newId;
        return newPacket;
    }

    void beep(uint16_t period)
    {
        sinceBeep = 0;
        M5.Beep.tone(4000);
        while (sinceBeep < period)
        {
            vTaskDelay(TICKS_5ms);
        }
        M5.Beep.mute();
    }

    void handleCommandPacket(CommandPacket *packet)
    {
        if (packet->command == COMMAND_SHORT_BEEP)
        {
            beep(50);
        }
        else if (packet->command == COMMAND_ENDLESS_BEEP)
        {
            beep(SIX_SECONDS);
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
