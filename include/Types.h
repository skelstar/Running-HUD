#pragma once
#include <Arduino.h>

const TickType_t TICKS_5ms = 5 / portTICK_PERIOD_MS;
const TickType_t TICKS_10ms = 10 / portTICK_PERIOD_MS;
const TickType_t TICKS_50ms = 50 / portTICK_PERIOD_MS;
const TickType_t TICKS_100ms = 100 / portTICK_PERIOD_MS;
const TickType_t TICKS_500ms = 500 / portTICK_PERIOD_MS;
const TickType_t TICKS_1s = 1000 / portTICK_PERIOD_MS;
const TickType_t TICKS_2s = 2000 / portTICK_PERIOD_MS;

const uint16_t ONE_SECONDS = 1000;
const uint16_t TWO_SECONDS = 2000;
const uint16_t THREE_SECONDS = 3000;
const uint16_t FOUR_SECONDS = 4000;
const uint16_t FIVE_SECONDS = 5000;

class InputPacket
{
public:
    unsigned long id = -1;
    uint8_t input;
    uint8_t event;
};

enum Command
{
    COMMAND_NOP,
    COMMAND_BELOW_ZONE,
    COMMAND_IN_ZONE,
    COMMAND_ABOVE_ZONE,
    COMMAND_CYCLE_BRIGHTNESS,
    COMMAND_ZONE_CHANGE,
    COMMAND_FLASH_RED_LED,
    COMMAND_DISCONNECTED,
};

class CommandPacket
{
public:
    unsigned long id = -1;
    Command command;
};

enum InputOption
{
    ACC_BTN,
    MAIN_BTN,
    RST_BTN,
    ACCEL,
    CLIP_DETECT,
};

char const *getInputOption(InputOption option)
{
    switch (option)
    {
    case ACC_BTN:
        return "ACC_BTN";
    case MAIN_BTN:
        return "MAIN_BTN";
    case RST_BTN:
        return "RST_BTN";
    case ACCEL:
        return "ACCEL";
    case CLIP_DETECT:
        return "CLIP_DETECT";
    default:
        Serial.printf("ERROR: Unhandled option: %d \n", option);
        return "UNHANDLED";
    }
}

enum ButtonEvent
{
    CLICK,
    LONGCLICK,
    DOUBLE_TAP,
    DETECTED,
    NOT_DETECTED,
};

char const *getInputEvent(ButtonEvent event)
{
    switch (event)
    {
    case CLICK:
        return "CLICK";
    case LONGCLICK:
        return "LONGCLICK";
    case DOUBLE_TAP:
        return "DOUBLE_TAP";
    case DETECTED:
        return "DETECTED";
    case NOT_DETECTED:
        return "NOT_DETECTED";
    default:
        Serial.printf("ERROR: Unhandled event: %d\n", event);
        return "UNHANDLED";
    }
}

enum ConnectionStatus
{
    DISCONNECTED,
    CONNECTED,
    BELOW_ZONE,
    IN_ZONE,
    ABOVE_ZONE,
    HZ_1,
    HZ_2,
    HZ_3,
    HZ_4,
    HZ_5,
};

char const *getConnectionStatus(ConnectionStatus status)
{
    switch (status)
    {
    case DISCONNECTED:
        return "DISCONNECTED";
    case CONNECTED:
        return "CONNECTED";
    case BELOW_ZONE:
        return "BELOW_ZONE";
    case IN_ZONE:
        return "IN_ZONE";
    case ABOVE_ZONE:
        return "ABOVE_ZONE";
    case HZ_1:
        return "HZ_1";
    case HZ_2:
        return "HZ_2";
    case HZ_3:
        return "HZ_3";
    case HZ_4:
        return "HZ_4";
    case HZ_5:
        return "HZ_5";
    }
    return "ERROR: Unhandled status";
}
