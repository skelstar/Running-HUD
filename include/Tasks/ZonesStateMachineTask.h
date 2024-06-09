#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "Bluetooth.h"

#include "ZonesStateMachine.h"

namespace ZonesStateMachineTask
{
    void handlePacket(Bluetooth::Packet *packet);

    void task1(void *pvParameters)
    {
        unsigned long blePacketId = -1;

        Serial.println("ZoneStateMachineTask: Started");

        ZonesStateMachine::SetupFsm();

        while (1)
        {
            Bluetooth::Packet *blePacket = nullptr;

            if (xQueuePeek(xBluetoothQueue, &(blePacket), TICKS_50ms) == pdTRUE)
            {
                // Serial.printf("ZonesStateMachine rxd packet %d\n", blePacket->id);
                if (blePacket->id != blePacketId)
                {
                    blePacketId = blePacket->id;

                    handlePacket(blePacket);
                }
            }

            ZonesStateMachine::fsm.run(100);

            vTaskDelay(TICKS_50ms); // Delay for 500 ms
        }
    }

    void handlePacket(Bluetooth::Packet *packet)
    {
        Serial.printf("(ZonesTask) xBluetoothQueue rxd: %d\n", packet->status);

        switch (packet->status)
        {
        case (uint8_t)Bluetooth::ConnectionStatus::CONNECTED:
            ZonesStateMachine::fsm.trigger(ZonesStateMachine::Trigger::ZONE_BELOW);
            Serial.printf("(ZonesTask) xBluetoothQueue triggered: STATE_BELOW_ZONE\n");
            break;
        case (uint8_t)Bluetooth::ConnectionStatus::DISCONNECTED:
            ZonesStateMachine::fsm.trigger(ZonesStateMachine::Trigger::BLE_DISCONNECTED);
            Serial.printf("(ZonesTask) xBluetoothQueue triggered: BLE_DISCONNECTED\n");
            break;
        }
    }
}
