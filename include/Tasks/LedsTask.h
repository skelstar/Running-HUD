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

    void handleButtonPacket(InputPacket *packet);
    void handlePacket(Bluetooth::Packet *packet);

    void task1(void *pvParameters)
    {
        unsigned long blePacketId = -1;
        unsigned long buttonPacketId = -1;

        Serial.printf("%s: Started\n", taskName);

        Leds::hudLed.begin();
        Leds::hudLed.show();

        Leds::setBrightness(Leds::BRIGHT_LOW);

        Leds::SetupFsm();

        selectedZone = SelectedZone::ZONE_TWO;

        while (1)
        {
            Bluetooth::Packet *blePacket = nullptr;

            if (xQueuePeek(xBluetoothQueue, &(blePacket), TICKS_50ms) == pdTRUE)
            {
                if (blePacket->id != blePacketId)
                {
                    blePacketId = blePacket->id;

                    handlePacket(blePacket);
                }
            }

            InputPacket *buttonPacket = nullptr;
            if (xQueuePeek(xInputsQueue, &(buttonPacket), TICKS_50ms) == pdTRUE)
            {
                if (buttonPacket->id != buttonPacketId)
                {
                    buttonPacketId = buttonPacket->id;

                    handleButtonPacket(buttonPacket);
                }
            }

            Leds::fsm.run(50);

            vTaskDelay(TICKS_50ms); // Delay for 500 ms
        }
    }

    void handleButtonPacket(InputPacket *packet)
    {
        switch (packet->button)
        {
        case ButtonOption::MAIN_BTN:
            switch (packet->event)
            {
            case ButtonEvent::CLICK:
                // Serial.printf("LedTask Main Btn click\n");
                Leds::cycleBrightnessUp();
                Leds::fsm.trigger(Leds::TR_CYCLE_BRIGHTNESS);
                break;
            case ButtonEvent::LONGCLICK:
                // Serial.printf("(LedsTask) xInputsQueue rxd: ACC_BTN event: LONG_CLICK\n");
                selectedZone = selectedZone == ZONE_TWO ? ZONE_THREE : ZONE_TWO;
                Leds::fsm.trigger(Leds::TR_ZONE_CHANGE);
                break;
            }
            break;
        case ButtonOption::RST_BTN:
            if (packet->event == ButtonEvent::LONGCLICK)
            {
                Leds::fsm.trigger(Leds::TR_POWER_DOWN);
            }
            break;
        }
    }

    uint8_t getEventForZone(uint8_t zone, uint8_t selectedZone)
    {
        if (zone == HZ2_TOP)
            return selectedZone == ZONE_TWO
                       ? Leds::TR_IN_ZONE
                       : Leds::TR_BELOW_ZONE;
        else if (zone == HZ3_TOP)
            return selectedZone == ZONE_THREE
                       ? Leds::TR_IN_ZONE
                       : Leds::TR_ABOVE_ZONE;
        else
            Serial.printf("Unhandled 'zone' value: %d \n", zone);
        return Leds::TR_ABOVE_ZONE;
    }

    void handlePacket(Bluetooth::Packet *packet)
    {
        switch (packet->status)
        {
        case Bluetooth::CONNECTED:
            if (packet->hr <= HZ1_TOP)
            {
                Leds::fsm.trigger(Leds::TR_BELOW_ZONE);
            }
            else if (packet->hr <= HZ2_TOP)
            {
                uint8_t event = getEventForZone(HZ2_TOP, selectedZone);
                Leds::fsm.trigger(event);
            }
            else if (packet->hr <= HZ3_TOP)
            {
                uint8_t event = getEventForZone(HZ3_TOP, selectedZone);
                Leds::fsm.trigger(event);
            }
            else if (packet->hr <= HZ4_TOP)
            {
                Leds::fsm.trigger(Leds::TR_ABOVE_ZONE);
            }
            else
            {
                Leds::fsm.trigger(Leds::TR_ABOVE_ZONE);
            }
            break;
        case Bluetooth::DISCONNECTED:
            Serial.printf("(LedsTask) xBluetoothQueue rxd: %s\n", packet->status ? "CONNECTED" : "DISCONNECTED");
            Leds::ledColour = Leds::COLOUR_BLUE;
            Leds::fsm.trigger(Leds::Trigger::TR_DISCONNECTED);
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