#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "Leds.h"

namespace LedsTask
{
    void handlePacket(Bluetooth::Packet *packet);

    void task1(void *pvParameters)
    {
        unsigned long blePacketId = -1;

        Serial.println("LedsTask: Started");

        Leds::hudLed.begin();
        Leds::hudLed.show();

        Leds::setBrightness(Leds::BRIGHT_MED);
        Leds::setLed(Leds::COLOUR_BLUE);

        Leds::SetupFsm();

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

            Leds::fsm.run(100);

            vTaskDelay(TICKS_50ms); // Delay for 500 ms
        }
    }

    void handlePacket(Bluetooth::Packet *packet)
    {
        switch (packet->status)
        {
        case Bluetooth::CONNECTED:
            Serial.printf("(LedsTask) xBluetoothQueue rxd: %s\n", packet->status ? "CONNECTED" : "DISCONNECTED");
            Leds::ledColour = Leds::COLOUR_GREEN;
            Leds::fsm.trigger(Leds::Trigger::SOLID);
            break;
        case Bluetooth::DISCONNECTED:
            Serial.printf("(LedsTask) xBluetoothQueue rxd: %s\n", packet->status ? "CONNECTED" : "DISCONNECTED");
            Leds::ledColour = Leds::COLOUR_BLUE;
            Leds::fsm.trigger(Leds::Trigger::START_FLASHING);
            break;
        case Bluetooth::BELOW_ZONE:
            Leds::ledColour = Leds::COLOUR_HEADLIGHT_WHITE;
            Leds::fsm.trigger(Leds::Trigger::SLOW_FLASHES);
            break;
        case Bluetooth::IN_ZONE:
            Leds::ledColour = Leds::COLOUR_GREEN;
            Leds::fsm.trigger(Leds::Trigger::SLOW_FLASHES);
            break;
        case Bluetooth::ABOVE_ZONE:
            Leds::ledColour = Leds::COLOUR_RED;
            Leds::fsm.trigger(Leds::Trigger::START_FLASHING);
            break;
        }
    }
}