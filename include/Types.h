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
    uint8_t button;
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
};

class CommandPacket
{
public:
    unsigned long id = -1;
    Command command;
};

enum ButtonOption
{
    ACC_BTN,
    MAIN_BTN,
    RST_BTN,
    ACCEL,
    CLIP_DETECT,
};

enum ButtonEvent
{
    CLICK,
    LONGCLICK,
    DOUBLE_TAP,
    DETECTED,
    NOT_DETECTED,
};
