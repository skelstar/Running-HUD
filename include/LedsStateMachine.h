#pragma once
#include <Arduino.h>
#include "Leds.h"
#include "Types.h"

namespace Leds
{
    SimpleFSM fsm;

    enum Trigger
    {
        TR_DISCONNECTED,
        TR_BELOW_ZONE,
        TR_IN_ZONE,
        TR_ABOVE_ZONE,
        TR_CYCLE_BRIGHTNESS,
        TR_ZONE_CHANGE,
        TR_POWER_DOWN,
    };

    enum StateName
    {
        STATE_DISCONNECTED,
        STATE_BELOW_ZONE,
        STATE_IN_ZONE,
        STATE_ABOVE_ZONE,
        STATE_CYCLE_BRIGHTNESS,
        STATE_ZONE_CHANGE,
        STATE_IDLE,
        STATE_POWER_DOWN,
    };

#define FLASH_5050_MS 500
#define FLASH_SHORT_MS 100
#define FLASH_LONG_MS 3000
#define FLASH_INFINITE -1

    elapsedMillis sinceEntered = 0;
    elapsedMillis sinceFlashed = 0;
    elapsedMillis sinceFlashWindow = 0;
    uint32_t ledColour = Leds::COLOUR_OFF;
    uint8_t flashCounter = 0;

    enum SelectedZone
    {
        ZONE_TWO,
        ZONE_THREE,
    } selectedZone;

    struct CurrentZone
    {
        uint8_t number;
        // uint32_t colour;
        FlashSchema schema;
        uint8_t numFlashes = 2;
        uint16_t flashWindow = TWO_SECONDS;
    } thisZone;

    void flashLed()
    {
        flashingState = !flashingState;
        setLed(flashingState ? ledColour : COLOUR_OFF);
        sinceFlashed = 0;
    }

    void setZoneFlashes(uint8_t numFlashes, uint16_t period, FlashSchema schema = FLASHES_EACH_SECOND)
    {
        thisZone.schema = schema;
        thisZone.numFlashes = numFlashes;
        thisZone.flashWindow = period;
    }

    void handleSchema(uint8_t schema)
    {
        switch (schema)
        {
        case FlashSchema::FLASHES_EACH_SECOND:
        case FlashSchema::FLASHES_ONE_OFF:
            if (sinceFlashed > 50 && flashCounter < thisZone.numFlashes)
            {
                flashLed();
                if (flashingState == 0)
                    flashCounter++;
            }
            // time to start flashes again (if flashes each second)?
            else if (sinceFlashWindow > thisZone.flashWindow &&
                     schema == FLASHES_EACH_SECOND)
            {
                sinceFlashWindow = 0;
                flashCounter = 0;
            }
            break;
        case FlashSchema::FIFTY_FIFTY:
            if (sinceFlashed > FLASH_5050_MS)
                flashLed();
            break;
        default:
            Serial.printf("Unhandled schema: %d\n", schema);
        }
    }

    void onEnter_disconnected()
    {
        Serial.printf("onEnter_disconnected()\n");
        ledColour = COLOUR_BLUE;
        setLed(ledColour);
        thisZone.schema = FlashSchema::FIFTY_FIFTY;
    }

    void handleSchema_OnState()
    {
        handleSchema(thisZone.schema);
    }

    void onEnter_belowZone()
    {
        Serial.printf("onEnter_belowZone() \n");
        ledColour = COLOUR_WHITE;
        Leds::setLed(ledColour);
        setZoneFlashes(2, TWO_SECONDS);
    }

    void onEnter_inZone()
    {
        Serial.printf("onEnter_inZone() \n");
        ledColour = COLOUR_GREEN;
        Leds::setLed(ledColour);
        setZoneFlashes(3, THREE_SECONDS, FLASHES_ONE_OFF);
    }

    void onEnter_aboveZone()
    {
        Serial.printf("onEnter_aboveZone() \n");
        ledColour = COLOUR_RED;
        Leds::setLed(ledColour);
        setZoneFlashes(2, ONE_SECONDS);
    }

    void onEnter_cycleBrightness()
    {
        Serial.printf("onEnter_cycleBrightness() \n");
        setLed(ledColour);
    }

    void onEnter_zoneChange()
    {
        Serial.printf("onEnter_zoneChange() \n");
        uint32_t col = selectedZone == SelectedZone::ZONE_TWO
                           ? COLOUR_GREEN
                           : COLOUR_RED;
        setLed(col);
    }

    void onEnter_idle() {}

    void onEnter_powerDown()
    {
        Serial.printf("onEnter_powerDown() \n");
        ledColour = COLOUR_RED;
        setZoneFlashes(8, TWO_SECONDS, FLASHES_ONE_OFF);
        Leds::setBrightness(BRIGHT_HIGH);
        Leds::setLed(ledColour);
    }

    State zone[] = {
        State("stateDisconnected", &onEnter_disconnected, &handleSchema_OnState),
        State("stateBelowZone", &onEnter_belowZone, &handleSchema_OnState),
        State("stateInZone", &onEnter_inZone, &handleSchema_OnState),
        State("stateAboveZone", &onEnter_aboveZone, &handleSchema_OnState),
        State("stateCycleBrightness", &onEnter_cycleBrightness),
        State("stateZoneChange", &onEnter_zoneChange),
        State("stateIdle", &onEnter_idle),
        State("statePowerDown", &onEnter_powerDown, &handleSchema_OnState)};

    void onRun()
    {
        sinceEntered = 0;
        flashCounter = 0;
        sinceFlashWindow = 0;
    }

    TimedTransition timedTransitions[] = {
        TimedTransition(&zone[STATE_CYCLE_BRIGHTNESS], &zone[STATE_IDLE], 500),
        TimedTransition(&zone[STATE_ZONE_CHANGE], &zone[STATE_IDLE], 1000),
    };

    Transition transitions[] = {
        // TR_IN_ZONE
        Transition(&zone[STATE_BELOW_ZONE], &zone[STATE_IN_ZONE], Trigger::TR_IN_ZONE, &onRun),
        Transition(&zone[STATE_ABOVE_ZONE], &zone[STATE_IN_ZONE], Trigger::TR_IN_ZONE, &onRun),
        Transition(&zone[STATE_DISCONNECTED], &zone[STATE_IN_ZONE], Trigger::TR_IN_ZONE, &onRun),
        Transition(&zone[STATE_IDLE], &zone[STATE_IN_ZONE], Trigger::TR_IN_ZONE, &onRun),

        // TR_ABOVE_ZONE
        Transition(&zone[STATE_IN_ZONE], &zone[STATE_ABOVE_ZONE], Trigger::TR_ABOVE_ZONE, &onRun),
        Transition(&zone[STATE_DISCONNECTED], &zone[STATE_ABOVE_ZONE], Trigger::TR_ABOVE_ZONE, &onRun),
        Transition(&zone[STATE_IDLE], &zone[STATE_ABOVE_ZONE], Trigger::TR_ABOVE_ZONE, &onRun),

        // TR_BELOW_ZONE
        Transition(&zone[STATE_IN_ZONE], &zone[STATE_BELOW_ZONE], Trigger::TR_BELOW_ZONE, &onRun),
        Transition(&zone[STATE_DISCONNECTED], &zone[STATE_BELOW_ZONE], Trigger::TR_BELOW_ZONE, &onRun),
        Transition(&zone[STATE_IDLE], &zone[STATE_BELOW_ZONE], Trigger::TR_BELOW_ZONE, &onRun),

        // TR_DISCONNECTED
        Transition(&zone[STATE_BELOW_ZONE], &zone[STATE_DISCONNECTED], Trigger::TR_DISCONNECTED, &onRun),
        Transition(&zone[STATE_IN_ZONE], &zone[STATE_DISCONNECTED], Trigger::TR_DISCONNECTED, &onRun),
        Transition(&zone[STATE_ABOVE_ZONE], &zone[STATE_DISCONNECTED], Trigger::TR_DISCONNECTED, &onRun),
        Transition(&zone[STATE_IDLE], &zone[STATE_DISCONNECTED], Trigger::TR_DISCONNECTED, &onRun),

        // -> TR_CYCLE_BRIGHTNESS
        Transition(&zone[STATE_DISCONNECTED], &zone[STATE_CYCLE_BRIGHTNESS], Trigger::TR_CYCLE_BRIGHTNESS, &onRun),
        Transition(&zone[STATE_BELOW_ZONE], &zone[STATE_CYCLE_BRIGHTNESS], Trigger::TR_CYCLE_BRIGHTNESS, &onRun),
        Transition(&zone[STATE_IN_ZONE], &zone[STATE_CYCLE_BRIGHTNESS], Trigger::TR_CYCLE_BRIGHTNESS, &onRun),
        Transition(&zone[STATE_ABOVE_ZONE], &zone[STATE_CYCLE_BRIGHTNESS], Trigger::TR_CYCLE_BRIGHTNESS, &onRun),

        // -> TR_ZONE_CHANGE
        Transition(&zone[STATE_BELOW_ZONE], &zone[STATE_ZONE_CHANGE], Trigger::TR_ZONE_CHANGE, &onRun),
        Transition(&zone[STATE_IN_ZONE], &zone[STATE_ZONE_CHANGE], Trigger::TR_ZONE_CHANGE, &onRun),
        Transition(&zone[STATE_ABOVE_ZONE], &zone[STATE_ZONE_CHANGE], Trigger::TR_ZONE_CHANGE, &onRun),

        // TR_POWER_DOWN
        Transition(&zone[STATE_DISCONNECTED], &zone[STATE_POWER_DOWN], Trigger::TR_POWER_DOWN, &onRun),
        Transition(&zone[STATE_BELOW_ZONE], &zone[STATE_POWER_DOWN], Trigger::TR_POWER_DOWN, &onRun),
        Transition(&zone[STATE_IN_ZONE], &zone[STATE_POWER_DOWN], Trigger::TR_POWER_DOWN, &onRun),
        Transition(&zone[STATE_ABOVE_ZONE], &zone[STATE_POWER_DOWN], Trigger::TR_POWER_DOWN, &onRun),
    };

    void SetupFsm()
    {
        fsm.add(transitions, sizeof(transitions) / sizeof(Transition));
        fsm.add(timedTransitions, sizeof(timedTransitions) / sizeof(TimedTransition));

        fsm.setInitialState(&zone[STATE_DISCONNECTED]);
    }
}