#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "Types.h"
#include "Leds.h"
#include "LedsStateMachine.h"

namespace Leds
{
    TaskHandle_t taskHandle = nullptr;
    const char *taskName = "LedsTask";

    void handleCommand(CommandPacket *packet);

    void task1(void *pvParameters)
    {
        unsigned long commandId = -1;

        Serial.printf("%s: Started\n", taskName);

        Leds::hudLed.begin();
        Leds::hudLed.show();

        Leds::setBrightness(Leds::BRIGHT_LOW);

        Leds::SetupFsm();

        while (1)
        {
            CommandPacket *commandPacket = nullptr;

            if (xQueuePeek(xCommandQueue, &(commandPacket), TICKS_50ms) == pdTRUE)
            {
                if (commandPacket->id != commandId)
                {
                    commandId = commandPacket->id;

                    handleCommand(commandPacket);
                }
            }

            fsm.run(5);

            vTaskDelay(TICKS_10ms); // Delay for 500 ms
        }
    }

    void handleCommand(CommandPacket *packet)
    {
        switch (packet->command)
        {
        case COMMAND_NOP:
            break;
        case COMMAND_BELOW_ZONE:
            fsm.trigger(TR_BELOW_ZONE);
            break;
        case COMMAND_IN_ZONE:
            fsm.trigger(TR_IN_ZONE);
            break;
        case COMMAND_ABOVE_ZONE:
            fsm.trigger(TR_ABOVE_ZONE);
            break;
        case COMMAND_ABOVE_ZONE_PLUS:
            fsm.trigger(TR_ABOVE_ZONE_PLUS);
            break;
        case COMMAND_CYCLE_BRIGHTNESS:
            cycleBrightnessUp();
            fsm.trigger(TR_CYCLE_BRIGHTNESS);
            break;
        case COMMAND_ZONE_CHANGE:
            fsm.trigger(TR_ZONE_CHANGE);
            break;
        case COMMAND_SET_CUSTOM_HR:
        case COMMAND_CLEAR_CUSTOM_HR:
            fsm.trigger(TR_CUSTOM_HEARTRATE);
            break;
        case COMMAND_POWERING_DOWN:
            fsm.trigger(TR_POWERING_DOWN);
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