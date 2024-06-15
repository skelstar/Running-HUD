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

    elapsedMillis sinceFlashedLed = 0;
    bool m5ledState = false;
    bool shouldFlashLed = true; // assume not connected straight away
    unsigned long blePacketId = 0;

    void task1(void *pvParameters)
    {
        Serial.printf("%s: Started\n", taskName);

        pinMode(M5_LED_PIN, OUTPUT);
        digitalWrite(M5_LED_PIN, HIGH); // off

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

            uint16_t blinkMs = m5ledState == 0 ? 50 : 3000;
            if (shouldFlashLed && sinceFlashedLed > blinkMs)
            {
                sinceFlashedLed = 0;
                m5ledState = !m5ledState;
                digitalWrite(M5_LED_PIN, m5ledState);
            }

            vTaskDelay(TICKS_50ms);
        }
    }

    void handlePacket(Bluetooth::Packet *packet)
    {
        shouldFlashLed = packet->status == Bluetooth::DISCONNECTED ||
                         (packet->status == Bluetooth::CONNECTED && packet->hr < HZ1_TOP);
    }
}
