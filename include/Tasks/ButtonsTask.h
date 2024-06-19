#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <Button2.h>
#include <M5StickCPlus.h>

#include "Types.h"

namespace ButtonsTask
{
#define LONGCLICK_MS 1000

    TaskHandle_t taskHandle = NULL;
    const char *taskName = "ButtonsTask";

    InputPacket packet;

    namespace ButtonAcc
    {
        Button2 button;

        void sendPacket(InputPacket *packet)
        {
            InputPacket *data;
            data = packet;
            xQueueSend(xInputsQueue, (void *)&data, TICKS_10ms);
        }

        void clickHandler(Button2 &btn)
        {
            packet.id++;
            packet.button = ButtonOption::ACC_BTN;
            packet.event = ButtonEvent::CLICK;

            sendPacket(&packet);
        }

        void longClickDetectedHandler(Button2 &btn)
        {
            packet.id++;
            packet.button = ButtonOption::ACC_BTN;
            packet.event = ButtonEvent::LONGCLICK;

            sendPacket(&packet);
        }

        void doubleTapHandler_cb(Button2 &btn)
        {
            packet.id++;
            packet.button = ButtonOption::ACC_BTN;
            packet.event = ButtonEvent::DOUBLE_TAP;

            sendPacket(&packet);
        }

        void initialise()
        {
            button.begin(ACC_BUTTON_PIN);
            button.setLongClickTime(LONGCLICK_MS);
            button.setDoubleClickTime(500); // default: BTN_DOUBLECLICK_MS = 300

            // handlers
            button.setClickHandler(&clickHandler);
            button.setLongClickDetectedHandler(&longClickDetectedHandler);
            button.setDoubleClickHandler(&doubleTapHandler_cb);
        }
    }

    namespace ButtonMain
    {
        Button2 button;

        void sendPacket(InputPacket *packet)
        {
            InputPacket *data;
            data = packet;
            xQueueSend(xInputsQueue, (void *)&data, TICKS_10ms);
        }

        void clickHandler(Button2 &btn)
        {
            // Serial.printf("Main Button clicked\n");
            packet.id++;
            packet.button = ButtonOption::MAIN_BTN;
            packet.event = CLICK;

            sendPacket(&packet);
        }

        void longClickDetectedHandler(Button2 &btn)
        {
            packet.id++;
            packet.button = ButtonOption::MAIN_BTN;
            packet.event = LONGCLICK;

            sendPacket(&packet);
        }

        void initialise()
        {
            button.begin(MAIN_BUTTON_PIN);
            button.setLongClickTime(LONGCLICK_MS);

            // handlers
            button.setClickHandler(&clickHandler);
            button.setLongClickDetectedHandler(&longClickDetectedHandler);
        }
    }

    namespace ButtonRst
    {
        void sendPacket(InputPacket *packet)
        {
            InputPacket *data;
            data = packet;
            xQueueSend(xInputsQueue, (void *)&data, TICKS_10ms);
        }

        void clickHandler()
        {
            Serial.printf("Rst Button clicked\n");

            esp_restart();
        }

        void longClickDetectedHandler()
        {
            Serial.printf("Rst Button long click\n");

            packet.id++;
            packet.button = ButtonOption::RST_BTN;
            packet.event = ButtonEvent::LONGCLICK;

            sendPacket(&packet);
        }

        void loop()
        {
            switch (M5.Axp.GetBtnPress())
            {
            case 0x02:
                clickHandler();
                break;
            case 0x01:
                longClickDetectedHandler();
                break;
            }
        }

        void initialise()
        {
            M5.begin();
            M5.Axp.begin();
        }
    }

    void task1(void *pvParameters)
    {
        Serial.println("ButtonsTask: Started");

        ButtonMain::initialise();
        ButtonAcc::initialise();
        // ButtonRst::initialise();

        while (1)
        {
            // update packet.id
            xQueuePeek(xInputsQueue, (void *)&packet, TICKS_50ms);

            ButtonMain::button.loop();
            ButtonAcc::button.loop();
            // ButtonRst::loop();

            vTaskDelay(TICKS_5ms);
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
