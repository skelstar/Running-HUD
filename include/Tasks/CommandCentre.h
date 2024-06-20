#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <elapsedMillis.h>
#include "Bluetooth.h"
#include "Types.h"

namespace CommandCentre
{
    TaskHandle_t taskHandle = NULL;
    const char *taskName = "CommandCentre";

    // Prototypes
    void handleBlePacket(Bluetooth::Packet *packet);
    void handleButtonPacket(InputPacket *packet);
    Command getCommandForZone(uint8_t zone, uint8_t selectedZone);
    void sendCommand(Command command);

    enum SelectedZone
    {
        ZONE_TWO,
        ZONE_THREE,
    } selectedZone;

    unsigned long blePacketId = -1;
    unsigned long inputPacketId = -1;
    unsigned long commandPktId = -1;

    CommandPacket commandPacket;

    void task1(void *pvParameters)
    {
        Serial.printf("%s: Started\n", taskName);

        while (1)
        {
            Bluetooth::Packet *blePacket = nullptr;

            if (xQueuePeek(xBluetoothQueue, &(blePacket), TICKS_50ms) == pdTRUE)
            {
                if (blePacket->id != blePacketId)
                {
                    blePacketId = blePacket->id;

                    handleBlePacket(blePacket);
                }
            }

            InputPacket *inputPacket = nullptr;
            if (xQueuePeek(xInputsQueue, &(inputPacket), TICKS_50ms) == pdTRUE)
            {
                if (inputPacket->id != inputPacketId)
                {
                    inputPacketId = inputPacket->id;

                    handleButtonPacket(inputPacket);
                }
            }

            vTaskDelay(TICKS_5ms);
        }
    }

    Command getCommandForZone(uint8_t zone, uint8_t selectedZone)
    {
        if (zone == HZ2_TOP)
            return selectedZone == ZONE_TWO
                       ? Command::COMMAND_IN_ZONE
                       : Command::COMMAND_BELOW_ZONE;
        else if (zone == HZ3_TOP)
            return selectedZone == ZONE_THREE
                       ? Command::COMMAND_IN_ZONE
                       : Command::COMMAND_ABOVE_ZONE;
        else
            Serial.printf("Unhandled 'zone' value: %d \n", zone);
        return Command::COMMAND_NOP;
    }

    void handleBlePacket(Bluetooth::Packet *packet)
    {
        Command command = COMMAND_NOP;

        switch (packet->status)
        {
        case Bluetooth::CONNECTED:
            if (packet->hr <= HZ1_TOP)
                command = Command::COMMAND_BELOW_ZONE;
            else if (packet->hr <= HZ2_TOP)
                command = getCommandForZone(HZ2_TOP, selectedZone);
            else if (packet->hr <= HZ3_TOP)
                command = getCommandForZone(HZ3_TOP, selectedZone);
            else if (packet->hr <= HZ4_TOP)
                command = Command::COMMAND_ABOVE_ZONE;
            else // HZ5
                command = Command::COMMAND_ABOVE_ZONE;
            sendCommand(command);
            break;

        case Bluetooth::DISCONNECTED:
            sendCommand(command);
            break;
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
                sendCommand(COMMAND_CYCLE_BRIGHTNESS);
                break;
            case ButtonEvent::LONGCLICK:
                // Serial.printf("(LedsTask) xInputsQueue rxd: ACC_BTN event: LONG_CLICK\n");
                selectedZone = selectedZone == ZONE_TWO ? ZONE_THREE : ZONE_TWO;
                sendCommand(COMMAND_ZONE_CHANGE);
                break;
            }
            break;
        }
    }

    void sendCommand(Command command)
    {
        commandPacket.id++;
        commandPacket.command = command;

        CommandPacket *data;
        data = &commandPacket;
        xQueueSend(xCommandQueue, (void *)&data, TICKS_10ms);
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