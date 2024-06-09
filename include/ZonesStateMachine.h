#pragma once
#include <SimpleFsm.h>

namespace ZonesStateMachine
{
	SimpleFSM fsm;

	enum StateID
	{
		ST_UNKNOWN = 0,
		ST_ONE,
	};

	enum Trigger
	{
		ZONE_UP,
		ZONE_DOWN,
		BLE_DISCONNECTED,
		ZONE_BELOW,
		ZONE_IN,
		ZONE_ABOVE,
	};

	enum StateName
	{
		STATE_BELOW_ZONE,
		STATE_IN_ZONE,
		STATE_ABOVE_ZONE,
		STATE_BLE_DISCONNECTED,
	};

	elapsedMillis sinceEntered = 0;
	elapsedMillis sinceFlashed = 0;
	uint32_t zoneColour = Leds::COLOUR_OFF;

	void on_enter_belowZone()
	{
		Serial.printf("In state: belowZone\n");

		Leds::ledColour = Leds::COLOUR_HEADLIGHT_WHITE;
		Leds::fsm.trigger(Leds::START_FLASHING);
	}

	void on_state_below_zone()
	{
		if (sinceEntered > 1000)
		{
			Leds::fsm.trigger(Leds::SOLID);
		}
	}

	void on_enter_inZone()
	{
		Serial.printf("In state: inZone\n");

		Leds::ledColour = Leds::COLOUR_GREEN;
		Leds::fsm.trigger(Leds::SOLID);
	}

	void on_enter_aboveZone()
	{
		Serial.printf("In state: aboveZone\n");

		Leds::ledColour = Leds::COLOUR_RED;
		Leds::fsm.trigger(Leds::START_FLASHING);
	}

	void on_state_above_zone()
	{
		if (sinceEntered > 1000)
		{
			Leds::fsm.trigger(Leds::SOLID);
		}
	}

	void on_enter_ble_disconnected()
	{
		Serial.printf("In state: ble_disconnected\n");

		Leds::ledColour = Leds::COLOUR_BLUE;
		Leds::fsm.trigger(Leds::START_FLASHING);
	}

	State zone[] = {
		State("stateBelowZone", &on_enter_belowZone, &on_state_below_zone),
		State("stateInZone", &on_enter_inZone),
		State("stateAboveZone", &on_enter_aboveZone, &on_state_above_zone),
		State("stateBleDisconnected", &on_enter_ble_disconnected),
	};

	void onRun()
	{
		sinceEntered = 0;
	}

	bool onGuard()
	{
		// Serial.printf("Checking onGuard()\n");
		return true;
	}

	Transition transitions[] = {
		// ZONE_IN
		Transition(&zone[STATE_BELOW_ZONE], &zone[STATE_IN_ZONE], Trigger::ZONE_IN, &onRun, "transition into zone", &onGuard),
		Transition(&zone[STATE_ABOVE_ZONE], &zone[STATE_IN_ZONE], Trigger::ZONE_IN, &onRun, "transition out of zone (high)", &onGuard),
		Transition(&zone[STATE_BLE_DISCONNECTED], &zone[STATE_IN_ZONE], Trigger::ZONE_IN, &onRun),

		// ZONE_BELOW
		Transition(&zone[STATE_IN_ZONE], &zone[STATE_BELOW_ZONE], Trigger::ZONE_BELOW, &onRun, "transition out of zone (high)", &onGuard),
		Transition(&zone[STATE_BLE_DISCONNECTED], &zone[STATE_BELOW_ZONE], Trigger::ZONE_BELOW, &onRun),

		// ZONE_ABOVE
		Transition(&zone[STATE_IN_ZONE], &zone[STATE_ABOVE_ZONE], Trigger::ZONE_ABOVE, &onRun, "transition out of zone (high)", &onGuard),
		Transition(&zone[STATE_BLE_DISCONNECTED], &zone[STATE_ABOVE_ZONE], Trigger::ZONE_ABOVE, &onRun),

		// BLE_DISCONNECTED
		Transition(&zone[STATE_BELOW_ZONE], &zone[STATE_BLE_DISCONNECTED], Trigger::BLE_DISCONNECTED, &onRun),
		Transition(&zone[STATE_IN_ZONE], &zone[STATE_BLE_DISCONNECTED], Trigger::BLE_DISCONNECTED, &onRun),
		Transition(&zone[STATE_ABOVE_ZONE], &zone[STATE_BLE_DISCONNECTED], Trigger::BLE_DISCONNECTED, &onRun),

	};

	void SetupFsm()
	{
		int num_transitions = sizeof(transitions) / sizeof(Transition);
		fsm.add(transitions, num_transitions);

		fsm.setInitialState(&zone[STATE_BLE_DISCONNECTED]);
	}
}