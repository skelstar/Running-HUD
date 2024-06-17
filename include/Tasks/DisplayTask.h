#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <elapsedMillis.h>
#include "Types.h"

#include <M5StickC.h>

namespace DisplayTask
{
    TaskHandle_t taskHandle = NULL;
    const char *taskName = "DisplayTask";

    // prototypes
    void handleButtonPacket(ButtonPacket *packet);

    unsigned long buttonPacketId = -1;
    bool displayOn = false;

    void task1(void *pvParameters)
    {
        Serial.printf("%s: Started\n", taskName);

        M5.begin();
        M5.Axp.SetLDO2(false);

        while (1)
        {
            ButtonPacket *buttonPacket = nullptr;
            if (xQueuePeek(xButtonQueue, &(buttonPacket), TICKS_50ms) == pdTRUE)
            {
                if (buttonPacket->id != buttonPacketId)
                {
                    buttonPacketId = buttonPacket->id;

                    handleButtonPacket(buttonPacket);
                }
            }

            vTaskDelay(TICKS_50ms);
        }
    }

    void handleButtonPacket(ButtonPacket *packet)
    {
        switch (packet->button)
        {
        case ButtonOption::ACC_BTN:
            // Serial.printf("(DisplayTask) xButtonQueue rxd: ACC_BTN event: %s\n", packet->event == CLICK ? "CLICK" : "OTHER EVENT");
            switch (packet->event)
            {
            case ButtonEvent::CLICK:
                break;
            case ButtonEvent::LONGCLICK:
                break;
            case ButtonEvent::DOUBLE_TAP:
                break;
            }
            break;
        case ButtonOption::MAIN_BTN:
            // Serial.printf("(DisplayTask) xButtonQueue rxd: MAIN_BTN event: %s\n", packet->event == CLICK ? "CLICK" : "OTHER EVENT");
            switch (packet->event)
            {
            case ButtonEvent::CLICK:
                break;
            case ButtonEvent::LONGCLICK:
                displayOn = !displayOn;
                M5.Axp.SetLDO2(displayOn);
                M5.Lcd.fillScreen(WHITE);
                break;
            case ButtonEvent::DOUBLE_TAP:
                break;
            }
            break;
        case ButtonOption::ACCEL:
            Serial.printf("(DisplayTask) xButtonQueue rxd: ACCEL event: %s\n", packet->event == DOUBLE_TAP ? "DOUBLE_TAP" : "OTHER EVENT");
            break;
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
