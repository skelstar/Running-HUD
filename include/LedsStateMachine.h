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
    };

    enum StateName
    {
        STATE_DISCONNECTED,
        STATE_BELOW_ZONE,
        STATE_IN_ZONE,
        STATE_ABOVE_ZONE,
        STATE_CYCLE_BRIGHTNESS,
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

    void setZoneFlashes(uint8_t numFlashes, uint16_t period)
    {
        thisZone.schema = FLASHES_IN_SECOND;
        thisZone.numFlashes = numFlashes;
        thisZone.flashWindow = period;
    }

    void handleSchema(uint8_t schema)
    {
        switch (schema)
        {
        case FlashSchema::FLASHES_IN_SECOND:
            if (sinceFlashed > 50 && flashCounter < thisZone.numFlashes)
            {
                flashLed();
                if (flashingState == 0)
                    flashCounter++;
            }
            // time to start flashes again?
            else if (sinceFlashWindow > thisZone.flashWindow)
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

    void disconnected_OnEnter()
    {
        Serial.printf("disconnected_OnEnter()\n");
        ledColour = COLOUR_BLUE;
        setLed(ledColour);
        thisZone.schema = FlashSchema::FIFTY_FIFTY;
    }

    void disconnected_OnState()
    {
        handleSchema(thisZone.schema);
    }

    void belowZone_OnEnter()
    {
        Serial.printf("belowZone_Onnter_below_zone() \n");
        ledColour = COLOUR_HEADLIGHT_WHITE;
        setZoneFlashes(2, TWO_SECONDS);
        Leds::setLed(ledColour);
    }

    void belowZone_OnState()
    {
        handleSchema(thisZone.schema);
    }

    void inZone_OnEnter()
    {
        Serial.printf("inZone_OnEnter() \n");
        ledColour = COLOUR_GREEN;
        setZoneFlashes(1, THREE_SECONDS);
        Leds::setLed(ledColour);
    }

    void inZone_OnState()
    {
        handleSchema(thisZone.schema);
    }

    void aboveZone_OnEnter()
    {
        Serial.printf("aboveZone_OnEnter() \n");
        ledColour = COLOUR_RED;
        setZoneFlashes(3, TWO_SECONDS);
        Leds::setLed(ledColour);
    }

    void aboveZone_OnState()
    {
        handleSchema(thisZone.schema);
    }

    void cycleBrightness_OnEnter()
    {
        Serial.printf("cycleBrightness_OnEnter() \n");
        setLed(ledColour);
    }

    void cycleBrightness_OnState()
    {
    }

    State zone[] = {
        State("stateDisconnected", &disconnected_OnEnter, &disconnected_OnState),
        State("stateBelowZone", &belowZone_OnEnter, &belowZone_OnState),
        State("stateInZone", &inZone_OnEnter, &inZone_OnState),
        State("stateAboveZone", &aboveZone_OnEnter, &aboveZone_OnState),
        State("stateCycleBrightness", &cycleBrightness_OnEnter, &cycleBrightness_OnState),
    };

    void onRun()
    {
        sinceEntered = 0;
        flashCounter = 0;
        sinceFlashWindow = 0;
    }

    TimedTransition timedTransitions[] = {
        // TR_CYCLE_BRIGHTNESS
        TimedTransition(&zone[STATE_CYCLE_BRIGHTNESS], &zone[STATE_DISCONNECTED], 500),
    };

    Transition transitions[] = {
        // going UP
        Transition(&zone[STATE_BELOW_ZONE], &zone[STATE_IN_ZONE], Trigger::TR_IN_ZONE, &onRun),
        Transition(&zone[STATE_IN_ZONE], &zone[STATE_ABOVE_ZONE], Trigger::TR_ABOVE_ZONE, &onRun),

        // going DOWN
        Transition(&zone[STATE_ABOVE_ZONE], &zone[STATE_IN_ZONE], Trigger::TR_IN_ZONE, &onRun),
        Transition(&zone[STATE_IN_ZONE], &zone[STATE_BELOW_ZONE], Trigger::TR_BELOW_ZONE, &onRun),

        // from disconnected
        Transition(&zone[STATE_DISCONNECTED], &zone[STATE_BELOW_ZONE], Trigger::TR_BELOW_ZONE, &onRun),
        Transition(&zone[STATE_DISCONNECTED], &zone[STATE_IN_ZONE], Trigger::TR_IN_ZONE, &onRun),
        Transition(&zone[STATE_DISCONNECTED], &zone[STATE_ABOVE_ZONE], Trigger::TR_ABOVE_ZONE, &onRun),

        // to disconnected
        Transition(&zone[STATE_BELOW_ZONE], &zone[STATE_DISCONNECTED], Trigger::TR_DISCONNECTED, &onRun),
        Transition(&zone[STATE_IN_ZONE], &zone[STATE_DISCONNECTED], Trigger::TR_DISCONNECTED, &onRun),
        Transition(&zone[STATE_ABOVE_ZONE], &zone[STATE_DISCONNECTED], Trigger::TR_DISCONNECTED, &onRun),

        // -> TR_CYCLE_BRIGHTNESS
        Transition(&zone[STATE_DISCONNECTED], &zone[STATE_CYCLE_BRIGHTNESS], Trigger::TR_CYCLE_BRIGHTNESS, &onRun),
        Transition(&zone[STATE_BELOW_ZONE], &zone[STATE_CYCLE_BRIGHTNESS], Trigger::TR_CYCLE_BRIGHTNESS, &onRun),
        Transition(&zone[STATE_IN_ZONE], &zone[STATE_CYCLE_BRIGHTNESS], Trigger::TR_CYCLE_BRIGHTNESS, &onRun),
        Transition(&zone[STATE_ABOVE_ZONE], &zone[STATE_CYCLE_BRIGHTNESS], Trigger::TR_CYCLE_BRIGHTNESS, &onRun),
    };

    void SetupFsm()
    {
        fsm.add(transitions, sizeof(transitions) / sizeof(Transition));
        fsm.add(timedTransitions, sizeof(timedTransitions) / sizeof(TimedTransition));

        fsm.setInitialState(&zone[STATE_DISCONNECTED]);
    }
}