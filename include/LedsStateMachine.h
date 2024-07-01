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
        TR_ABOVE_ZONE_PLUS,
        TR_CYCLE_BRIGHTNESS,
        TR_ZONE_CHANGE,
        TR_POWERING_DOWN,
        TR_CUSTOM_HEARTRATE,
    };

    enum StateName
    {
        STATE_DISCONNECTED,
        STATE_BELOW_ZONE,
        STATE_IN_ZONE,
        STATE_ABOVE_ZONE,
        STATE_ABOVE_ZONE_PLUS,
        STATE_CYCLE_BRIGHTNESS,
        STATE_ZONE_CHANGE,
        STATE_IDLE,
        STATE_POWER_DOWN,
        STATE_CUSTOM_HEART_RATE,
    };

#define FLASH_5050_MS 500
#define FLASH_SHORT_MS 100
#define FLASH_LONG_MS 3000
#define FLASH_INFINITE -1

    elapsedMillis sinceEntered = 0;
    elapsedMillis sinceFlashed = 0;
    elapsedMillis sinceFlashWindow = 0;
    uint8_t flashCounter = 0;

    struct CurrentZone
    {
        uint8_t number;
        // uint32_t colour;
        FlashSchema schema;
        uint8_t numFlashes = 2;
        uint16_t flashWindow = TWO_SECONDS;
    } thisZone;

#define DONT_UPDATE_CURRENT 0

    void toggleLed()
    {
        ledState = !ledState;
        setLed(ledState ? currentColour : COLOUR_OFF, DONT_UPDATE_CURRENT);
        sinceFlashed = 0;
    }

    void setZoneFlashes(uint8_t numFlashes, uint16_t period, FlashSchema schema = FLASHES_EACH_SECOND)
    {
        thisZone.schema = schema;
        thisZone.numFlashes = numFlashes;
        thisZone.flashWindow = period;
    }

    void setNoFlashes()
    {
        Leds::setLed(COLOUR_OFF);
        thisZone.schema = FlashSchema::FLASHES_NONE;
        // not sure if these matter
        thisZone.numFlashes = 1;
        thisZone.flashWindow = ONE_SECONDS;
    }

#define FLASH_ON_TIME 20
#define FLASH_OFF_TIME 100
#define LED_OFF 0
#define LED_ON 1

    void handleSchema(uint8_t schema)
    {
        if (schema == FLASHES_EACH_SECOND ||
            schema == FLASHES_ONE_OFF)
        {
            uint16_t waitPeriod = ledState == LED_ON
                                      ? FLASH_ON_TIME
                                      : FLASH_OFF_TIME;

            // toggle led if we need to
            if (sinceFlashed > waitPeriod && flashCounter < thisZone.numFlashes)
            {
                toggleLed();
                if (ledState == LED_OFF)
                    flashCounter++;
            }
            // reset counters if windown finished
            else if (schema == FLASHES_EACH_SECOND &&
                     sinceFlashWindow > thisZone.flashWindow)
            {
                sinceFlashWindow = 0;
                flashCounter = 0;
            }
        }
        else if (schema == FIFTY_FIFTY)
        {
            if (sinceFlashed > FLASH_5050_MS)
                toggleLed();
        }
        else if (schema == FLASHES_NONE)
        {
            setLed(COLOUR_OFF);
        }
    }

#define NUM_FLASHES_BELOW_ZONE 2
#define NUM_FLASHES_ABOVE_ZONE 1
#define NUM_FLASHES_ABOVE_ZONE_PLUS 3

    void onEnter_disconnected()
    {
        Serial.printf("onEnter_disconnected()\n");
        setLed(COLOUR_BLUE);
        thisZone.schema = FlashSchema::FIFTY_FIFTY;
    }

    void handleSchema_OnState()
    {
        handleSchema(thisZone.schema);
    }

    void onEnter_belowZone()
    {
        Serial.printf("onEnter_belowZone() \n");
        Leds::setLed(COLOUR_WHITE);
        setZoneFlashes(NUM_FLASHES_BELOW_ZONE, TWO_SECONDS);
    }

    void onEnter_inZone()
    {
        Serial.printf("onEnter_inZone() \n");
        setNoFlashes();
    }

    void onEnter_aboveZone()
    {
        Serial.printf("onEnter_aboveZone() \n");
        Leds::setLed(COLOUR_WHITE);
        setZoneFlashes(NUM_FLASHES_ABOVE_ZONE, ONE_AND_HALF_SECONDS);
    }

    void onEnter_aboveZonePlus()
    {
        Serial.printf("onEnter_aboveZonePlus() \n");
        Leds::setLed(COLOUR_WHITE);
        setZoneFlashes(NUM_FLASHES_ABOVE_ZONE_PLUS, TWO_THIRDS_SECONDS);
    }

    void onEnter_cycleBrightness()
    {
        Serial.printf("onEnter_cycleBrightness() \n");
        setLed(currentColour);
    }

    void onEnter_zoneChange()
    {
        Serial.printf("onEnter_zoneChange() \n");
        setLed(COLOUR_YELLOW);
    }

    void onEnter_idle() {}

    void onEnter_powerDown()
    {
        Serial.printf("onEnter_powerDown() \n");
        setBrightness(Brightness::BRIGHT_HIGHER);
        Leds::setLed(COLOUR_RED);
        setZoneFlashes(8, TWO_SECONDS, FLASHES_ONE_OFF);
        Leds::setBrightness(BRIGHT_HIGH);
    }

    void onEnter_customHeartRate()
    {
        Serial.printf("onEnter_customHeartRate() \n");
        _oldBrightness = _brightness;
        setBrightness(Brightness::BRIGHT_HIGHER);
        Leds::setLed(COLOUR_BLUE);
        setZoneFlashes(8, ONE_SECONDS, FLASHES_ONE_OFF);
    }

    void onExit_customerHeartRate()
    {
        setBrightness(_oldBrightness);
    }

    State zone[] = {
        State("stateDisconnected", &onEnter_disconnected, &handleSchema_OnState),
        State("stateBelowZone", &onEnter_belowZone, &handleSchema_OnState),
        State("stateInZone", &onEnter_inZone, &handleSchema_OnState),
        State("stateAboveZone", &onEnter_aboveZone, &handleSchema_OnState),
        State("stateAboveZonePlus", &onEnter_aboveZonePlus, &handleSchema_OnState),
        State("stateCycleBrightness", &onEnter_cycleBrightness),
        State("stateZoneChange", &onEnter_zoneChange),
        State("stateIdle", &onEnter_idle),
        State("statePowerDown", &onEnter_powerDown, &handleSchema_OnState),
        State("stateCustomHeartRate", &onEnter_customHeartRate, &handleSchema_OnState, &onExit_customerHeartRate),
    };

    void onRun()
    {
        sinceEntered = 0;
        flashCounter = 0;
        sinceFlashWindow = 0;
    }

    TimedTransition timedTransitions[] = {
        TimedTransition(&zone[STATE_CYCLE_BRIGHTNESS], &zone[STATE_IDLE], 500),
        TimedTransition(&zone[STATE_ZONE_CHANGE], &zone[STATE_IDLE], ONE_SECONDS),
        TimedTransition(&zone[STATE_CUSTOM_HEART_RATE], &zone[STATE_IDLE], ONE_SECONDS),
    };

    Transition transitions[] = {
        // TR_IN_ZONE
        Transition(&zone[STATE_BELOW_ZONE], &zone[STATE_IN_ZONE], Trigger::TR_IN_ZONE, &onRun),
        Transition(&zone[STATE_ABOVE_ZONE], &zone[STATE_IN_ZONE], Trigger::TR_IN_ZONE, &onRun),
        Transition(&zone[STATE_ABOVE_ZONE_PLUS], &zone[STATE_IN_ZONE], Trigger::TR_IN_ZONE, &onRun),
        Transition(&zone[STATE_DISCONNECTED], &zone[STATE_IN_ZONE], Trigger::TR_IN_ZONE, &onRun),
        Transition(&zone[STATE_IDLE], &zone[STATE_IN_ZONE], Trigger::TR_IN_ZONE, &onRun),

        // TR_ABOVE_ZONE
        Transition(&zone[STATE_IN_ZONE], &zone[STATE_ABOVE_ZONE], Trigger::TR_ABOVE_ZONE, &onRun),
        Transition(&zone[STATE_ABOVE_ZONE_PLUS], &zone[STATE_ABOVE_ZONE], Trigger::TR_ABOVE_ZONE, &onRun),
        Transition(&zone[STATE_DISCONNECTED], &zone[STATE_ABOVE_ZONE], Trigger::TR_ABOVE_ZONE, &onRun),
        Transition(&zone[STATE_IDLE], &zone[STATE_ABOVE_ZONE], Trigger::TR_ABOVE_ZONE, &onRun),

        // TR_ABOVE_ZONE_PLUS
        Transition(&zone[STATE_IN_ZONE], &zone[STATE_ABOVE_ZONE_PLUS], Trigger::TR_ABOVE_ZONE_PLUS, &onRun),
        Transition(&zone[STATE_ABOVE_ZONE], &zone[STATE_ABOVE_ZONE_PLUS], Trigger::TR_ABOVE_ZONE_PLUS, &onRun),
        Transition(&zone[STATE_DISCONNECTED], &zone[STATE_ABOVE_ZONE_PLUS], Trigger::TR_ABOVE_ZONE_PLUS, &onRun),
        Transition(&zone[STATE_IDLE], &zone[STATE_ABOVE_ZONE_PLUS], Trigger::TR_ABOVE_ZONE_PLUS, &onRun),

        // TR_BELOW_ZONE
        Transition(&zone[STATE_IN_ZONE], &zone[STATE_BELOW_ZONE], Trigger::TR_BELOW_ZONE, &onRun),
        Transition(&zone[STATE_DISCONNECTED], &zone[STATE_BELOW_ZONE], Trigger::TR_BELOW_ZONE, &onRun),
        Transition(&zone[STATE_IDLE], &zone[STATE_BELOW_ZONE], Trigger::TR_BELOW_ZONE, &onRun),

        // TR_DISCONNECTED
        Transition(&zone[STATE_BELOW_ZONE], &zone[STATE_DISCONNECTED], Trigger::TR_DISCONNECTED, &onRun),
        Transition(&zone[STATE_IN_ZONE], &zone[STATE_DISCONNECTED], Trigger::TR_DISCONNECTED, &onRun),
        Transition(&zone[STATE_ABOVE_ZONE], &zone[STATE_DISCONNECTED], Trigger::TR_DISCONNECTED, &onRun),
        Transition(&zone[STATE_ABOVE_ZONE_PLUS], &zone[STATE_DISCONNECTED], Trigger::TR_DISCONNECTED, &onRun),
        Transition(&zone[STATE_IDLE], &zone[STATE_DISCONNECTED], Trigger::TR_DISCONNECTED, &onRun),

        // -> TR_CYCLE_BRIGHTNESS
        Transition(&zone[STATE_DISCONNECTED], &zone[STATE_CYCLE_BRIGHTNESS], Trigger::TR_CYCLE_BRIGHTNESS, &onRun),
        Transition(&zone[STATE_BELOW_ZONE], &zone[STATE_CYCLE_BRIGHTNESS], Trigger::TR_CYCLE_BRIGHTNESS, &onRun),
        Transition(&zone[STATE_IN_ZONE], &zone[STATE_CYCLE_BRIGHTNESS], Trigger::TR_CYCLE_BRIGHTNESS, &onRun),
        Transition(&zone[STATE_ABOVE_ZONE], &zone[STATE_CYCLE_BRIGHTNESS], Trigger::TR_CYCLE_BRIGHTNESS, &onRun),
        Transition(&zone[STATE_ABOVE_ZONE_PLUS], &zone[STATE_CYCLE_BRIGHTNESS], Trigger::TR_CYCLE_BRIGHTNESS, &onRun),

        // -> TR_ZONE_CHANGE
        Transition(&zone[STATE_BELOW_ZONE], &zone[STATE_ZONE_CHANGE], Trigger::TR_ZONE_CHANGE, &onRun),
        Transition(&zone[STATE_IN_ZONE], &zone[STATE_ZONE_CHANGE], Trigger::TR_ZONE_CHANGE, &onRun),
        Transition(&zone[STATE_ABOVE_ZONE], &zone[STATE_ZONE_CHANGE], Trigger::TR_ZONE_CHANGE, &onRun),
        Transition(&zone[STATE_ABOVE_ZONE_PLUS], &zone[STATE_ZONE_CHANGE], Trigger::TR_ZONE_CHANGE, &onRun),

        // TR_POWERING_DOWN
        Transition(&zone[STATE_DISCONNECTED], &zone[STATE_POWER_DOWN], Trigger::TR_POWERING_DOWN, &onRun),
        Transition(&zone[STATE_BELOW_ZONE], &zone[STATE_POWER_DOWN], Trigger::TR_POWERING_DOWN, &onRun),
        Transition(&zone[STATE_IN_ZONE], &zone[STATE_POWER_DOWN], Trigger::TR_POWERING_DOWN, &onRun),
        Transition(&zone[STATE_ABOVE_ZONE], &zone[STATE_POWER_DOWN], Trigger::TR_POWERING_DOWN, &onRun),
        Transition(&zone[STATE_ABOVE_ZONE_PLUS], &zone[STATE_POWER_DOWN], Trigger::TR_POWERING_DOWN, &onRun),

        // TR_CUSTOM_HEARTRATE
        Transition(&zone[STATE_DISCONNECTED], &zone[STATE_CUSTOM_HEART_RATE], Trigger::TR_CUSTOM_HEARTRATE, &onRun),
        Transition(&zone[STATE_BELOW_ZONE], &zone[STATE_CUSTOM_HEART_RATE], Trigger::TR_CUSTOM_HEARTRATE, &onRun),
        Transition(&zone[STATE_IN_ZONE], &zone[STATE_CUSTOM_HEART_RATE], Trigger::TR_CUSTOM_HEARTRATE, &onRun),
        Transition(&zone[STATE_ABOVE_ZONE], &zone[STATE_CUSTOM_HEART_RATE], Trigger::TR_CUSTOM_HEARTRATE, &onRun),
        Transition(&zone[STATE_ABOVE_ZONE_PLUS], &zone[STATE_CUSTOM_HEART_RATE], Trigger::TR_CUSTOM_HEARTRATE, &onRun),
    };

    void SetupFsm()
    {
        fsm.add(transitions, sizeof(transitions) / sizeof(Transition));
        fsm.add(timedTransitions, sizeof(timedTransitions) / sizeof(TimedTransition));

        fsm.setInitialState(&zone[STATE_DISCONNECTED]);
    }
}