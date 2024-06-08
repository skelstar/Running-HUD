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
	};

	elapsedMillis sinceEntered = 0;
	elapsedMillis sinceFlashed = 0;
	uint32_t zoneColour = Leds::COLOUR_OFF;

	void enter_state_zone0()
	{
		Serial.printf("In state: zone0\n");
		sinceEntered = 0;
		zoneColour = Leds::COLOUR_GREY;
		Leds::setindicator(zoneColour);
	}

	void enter_state_zone1()
	{
		Serial.printf("In state: zone1\n");
		sinceEntered = 0;
		zoneColour = Leds::COLOUR_GREY;
		Leds::setindicator(zoneColour);
	}

	void enter_state_zone2()
	{
		Serial.printf("In state: zone2\n");
		sinceEntered = 0;
		zoneColour = Leds::COLOUR_WHITE;
		Leds::setindicator(zoneColour);
	}

	void enter_state_zone3()
	{
		Serial.printf("In state: zone3\n");
		sinceEntered = 0;
		zoneColour = Leds::COLOUR_YELLOW;
		Leds::setindicator(zoneColour);
	}

	void enter_state_zone4()
	{
		Serial.printf("In state: zone4\n");
		sinceEntered = 0;
		zoneColour = Leds::COLOUR_RED;
		Leds::setindicator(zoneColour);
	}

	void enter_state_zone5()
	{
		sinceEntered = 0;
		zoneColour = Leds::COLOUR_DARK_RED;
		Leds::setindicator(zoneColour);
	}

	void on_state()
	{
		// flash for first 2 beats?
	}

	State zone[] = {
			State("stateZone0", &enter_state_zone0, &on_state),
			State("stateZone1", &enter_state_zone1, &on_state),
			State("stateZone2", &enter_state_zone2, &on_state),
			State("stateZone3", &enter_state_zone3, &on_state),
			State("stateZone4", &enter_state_zone4, &on_state),
			State("stateZone5", &enter_state_zone5, &on_state),
	};

	void onRun()
	{
		if (fsm.getPreviousState() != nullptr)
			Serial.printf("onRun called (passed onGuard)\n- from state: %s \n- previous state: %s\n",
										fsm.getState()->getName(), // gets state before transition
										fsm.getPreviousState()->getName());
	}

	bool onGuard()
	{
		// Serial.printf("onGuard called");
		return true;
	}

	Transition transitions[] = {
			Transition(&zone[0], &zone[1], Trigger::ZONE_UP, &onRun, "transition zone 0 to 1", &onGuard),

			Transition(&zone[1], &zone[2], Trigger::ZONE_UP, &onRun, "transition zone 1 to 2", &onGuard),
			Transition(&zone[2], &zone[3], Trigger::ZONE_UP, &onRun, "transition zone 0 to 1", &onGuard),
			Transition(&zone[3], &zone[4], Trigger::ZONE_UP, &onRun, "transition zone 0 to 1", &onGuard),
			Transition(&zone[4], &zone[5], Trigger::ZONE_UP, &onRun, "transition zone 0 to 1", &onGuard),

			Transition(&zone[5], &zone[4], Trigger::ZONE_DOWN, &onRun, "transition zone 0 to 1", &onGuard),
			Transition(&zone[4], &zone[3], Trigger::ZONE_DOWN, &onRun, "transition zone 0 to 1", &onGuard),
			Transition(&zone[3], &zone[2], Trigger::ZONE_DOWN, &onRun, "transition zone 0 to 1", &onGuard),
			Transition(&zone[2], &zone[1], Trigger::ZONE_DOWN, &onRun, "transition zone 0 to 1", &onGuard),
	};

	void SetupFsm()
	{
		int num_transitions = sizeof(transitions) / sizeof(Transition);
		fsm.add(transitions, num_transitions);

		fsm.setInitialState(&zone[0]);
	}
}