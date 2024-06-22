#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <elapsedMillis.h>
#include "Types.h"

#include <M5StickCPlus.h>

namespace DisplayTask
{
    TaskHandle_t taskHandle = NULL;
    const char *taskName = "DisplayTask";

    // prototypes
    void handleButtonPacket(InputPacket *packet);
    void handleCommandPacket(CommandPacket *packet);
    void initScreen();
    void showStartupScreen();

    unsigned long buttonPacketId = -1;
    unsigned long commandPacketId = -1;
    bool displayOn = false;
    elapsedMillis sinceShowInfoCommand = THREE_SECONDS;

    void task1(void *pvParameters)
    {
        Serial.printf("%s: Started\n", taskName);

        M5.Axp.SetLDO2(false);
        initScreen();

        showStartupScreen();

        M5.Axp.SetLDO2(false); // turn off

        while (1)
        {
            CommandPacket *commandPacket = nullptr;
            if (xQueuePeek(xCommandQueue, &(commandPacket), TICKS_50ms) == pdTRUE &&
                commandPacket->id != commandPacketId)
            {
                commandPacketId = commandPacket->id;

                handleCommandPacket(commandPacket);
            }

            // if (sinceShowInfoCommand > TWO_SECONDS && displayOn)
            // {
            //     M5.Axp.SetLDO2(false);
            //     displayOn = false;
            // }

            vTaskDelay(TICKS_50ms);
        }
    }

    void handleCommandPacket(CommandPacket *packet)
    {
        if (packet->command == COMMAND_SHOW_INFO_SCREEN)
        {
            sinceShowInfoCommand = 0;
        }
    }

    void initScreen()
    {
        M5.Lcd.setRotation(3);
        M5.Lcd.fillScreen(BLACK);
        vTaskDelay(TICKS_500ms);
    }

    void showStartupScreen()
    {
        M5.Lcd.setCursor(5, 8);
        M5.Lcd.setTextColor(WHITE);
        M5.Lcd.setTextSize(2);
        M5.Lcd.printf("Main C: Bright");
        M5.Lcd.setCursor(5, 23);
        M5.Lcd.printf("Main L: Zone");

        vTaskDelay(TICKS_50ms);
        M5.Axp.SetLDO2(true); // turn on
        vTaskDelay(TICKS_1s);
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
