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
        unsigned long buttonPacketId = -1;

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

            Leds::fsm.run(50);

            vTaskDelay(TICKS_50ms); // Delay for 500 ms
        }
    }

    void handleCommand(CommandPacket *packet)
    {
        switch (packet->command)
        {
        case COMMAND_NOP:
            break;
        case COMMAND_BELOW_ZONE:
            Leds::fsm.trigger(TR_BELOW_ZONE);
            break;
        case COMMAND_IN_ZONE:
            Leds::fsm.trigger(TR_IN_ZONE);
            break;
        case COMMAND_ABOVE_ZONE:
            Leds::fsm.trigger(TR_ABOVE_ZONE);
            break;
        case COMMAND_CYCLE_BRIGHTNESS:
            cycleBrightnessUp();
            Leds::fsm.trigger(TR_CYCLE_BRIGHTNESS);
            break;
        case COMMAND_ZONE_CHANGE:
            Leds::fsm.trigger(TR_ZONE_CHANGE);
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