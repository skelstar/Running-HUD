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
    void handleInputEvent(InputPacket *packet);
    void handleClipDetectPacket(ClipDetectPacket *packet);
    void sendCommand(Command command);
    uint8_t getBottomOfSelectedZone();
    uint8_t getTopOfSelectedZone();

    enum SelectedZone
    {
        ZONE_TWO_IS_IN_ZONE,
        ZONE_THREE_IS_IN_ZONE,
    } selectedZone;

#define NO_CUSTOM_HR 0

    unsigned long blePacketId = -1;
    unsigned long inputPacketId = -1;
    unsigned long clipPacketId = -1;
    unsigned long commandPktId = -1;
    uint8_t _customHeartRate = 0;
    uint8_t _currentHr = 0;
    bool _bleConnected = false;
    uint8_t topOfInZone;
    uint8_t bottomOfInZone;

    CommandPacket commandPacket;

    void task1(void *pvParameters)
    {
        Serial.printf("%s: Started\n", taskName);

        selectedZone = ZONE_TWO_IS_IN_ZONE;
        topOfInZone = getTopOfSelectedZone();
        bottomOfInZone = getBottomOfSelectedZone();

        while (1)
        {
            Bluetooth::Packet *blePacket = nullptr;

            if (xQueuePeek(xBluetoothQueue, &(blePacket), TICKS_50ms) == pdTRUE)
            {
                if (blePacket->id != blePacketId)
                {
                    blePacketId = blePacket->id;
                    _currentHr = blePacket->hr;

                    handleBlePacket(blePacket);
                }
            }

            InputPacket *inputPacket = nullptr;
            if (xQueuePeek(xInputsQueue, &(inputPacket), TICKS_50ms) == pdTRUE &&
                inputPacket->id != inputPacketId)
            {
                inputPacketId = inputPacket->id;

                handleInputEvent(inputPacket);
            }

            ClipDetectPacket *clipPacket = nullptr;
            if (xQueuePeek(xClipDetectQueue, &(clipPacket), TICKS_50ms) == pdTRUE &&
                clipPacket->id != clipPacketId)
            {
                clipPacketId = clipPacket->id;

                handleClipDetectPacket(clipPacket);
            }

            vTaskDelay(TICKS_1ms);
        }
    }

    void sendCommandCustomHr(uint8_t hr)
    {
        Command command = hr <= _customHeartRate - 10
                              ? COMMAND_BELOW_ZONE
                          : hr <= _customHeartRate ? COMMAND_IN_ZONE
                                                   : COMMAND_ABOVE_ZONE;
        Serial.printf("(Custom) Sending command: %s (for custom: %d -> %d) \n",
                      getCommand(command), _customHeartRate - 10, _customHeartRate);
        sendCommand(command);
    }

    uint8_t getBottomOfSelectedZone()
    {
        return selectedZone == ZONE_TWO_IS_IN_ZONE ? HZ1_TOP : HZ2_TOP;
    }

    uint8_t getTopOfSelectedZone()
    {
        return selectedZone == ZONE_TWO_IS_IN_ZONE ? HZ2_TOP : HZ3_TOP;
    }

    void handleBlePacket(Bluetooth::Packet *packet)
    {
        Command command = COMMAND_NOP;

        switch (packet->status)
        {
        case ConnectionStatus::CONNECTED:
            _bleConnected = true;
            if (_customHeartRate != NO_CUSTOM_HR)
            {
                sendCommandCustomHr(packet->hr);
            }
            else if (packet->hr <= bottomOfInZone)
            {
                sendCommand(COMMAND_BELOW_ZONE);
            }
            else if (packet->hr <= topOfInZone)
            {
                sendCommand(COMMAND_IN_ZONE);
            }
            else if (packet->hr <= topOfInZone + 3)
            {
                sendCommand(COMMAND_ABOVE_ZONE);
            }
            else
            {
                sendCommand(COMMAND_ABOVE_ZONE_PLUS);
            }
            break;

        case ConnectionStatus::DISCONNECTED:
            _bleConnected = false;
            sendCommand(COMMAND_DISCONNECTED);
            break;
        }
    }

    void handleInputEvent(InputPacket *packet)
    {
        if (packet->input == MAIN_BTN)
        {
            switch (packet->event)
            {
            case ButtonEvent::CLICK:
                // Serial.printf("CommandCentre: LedTask Main Btn click\n");
                sendCommand(COMMAND_CYCLE_BRIGHTNESS);
                break;
            case ButtonEvent::LONGCLICK:
                // Serial.printf("(LedsTask) xInputsQueue rxd: ACC_BTN event: LONG_CLICK\n");
                selectedZone = selectedZone == ZONE_TWO_IS_IN_ZONE ? ZONE_THREE_IS_IN_ZONE : ZONE_TWO_IS_IN_ZONE;
                topOfInZone = getTopOfSelectedZone();
                bottomOfInZone = getBottomOfSelectedZone();
                sendCommand(COMMAND_ZONE_CHANGE);
                break;
            case ButtonEvent::DOUBLE_TAP:
                if (_bleConnected)
                {
                    _customHeartRate = _currentHr;
                    Serial.printf("Setting top of HRZ to custom value: %dbpm \n", _customHeartRate);
                    sendCommand(COMMAND_SET_CUSTOM_HR);
                }
                break;
            }
        }
        else if (packet->input == RST_BTN)
        {
            if (packet->event == CLICK)
                esp_restart();
            if (packet->event == LONGCLICK)
            {
                sendCommand(COMMAND_POWERING_DOWN);
            }
        }
        else if (packet->input == CONTROLLER_BTN)
        {
            switch (packet->event)
            {
            case ControllerAction::Click:
                Serial.printf("CommandCentre: CONTROLLER_BTN Click\n");
                sendCommand(COMMAND_CYCLE_BRIGHTNESS);
                break;
            }
        }
    }

    void handleClipDetectPacket(ClipDetectPacket *packet)
    {
        if (packet->status == CLIP_NOT_DETECTED)
        {
            sendCommand(COMMAND_FLASH_RED_LED);
        }
    }

    void sendCommand(Command command)
    {
        commandPacket.id++;
        commandPacket.command = command;

        CommandPacket *data;
        data = &commandPacket;
        xQueueSendToFront(xCommandQueue, (void *)&data, TICKS_50ms);
        // Serial.printf("Command %s sent\n", getCommand(command));
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
