// #include <Fsm.h>
#include <elapsedMillis.h>
#include <Adafruit_NeoPixel.h>

#define PIXEL_COUNT 1
#define BRIGHT_MAX 256

namespace Leds
{
	Adafruit_NeoPixel hudLed = Adafruit_NeoPixel(PIXEL_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);

	uint32_t COLOUR_OFF = hudLed.Color(0, 0, 0);
	uint32_t COLOUR_GREY = hudLed.Color(10, 10, 10);
	uint32_t COLOUR_WHITE = hudLed.Color(100, 100, 100);
	uint32_t COLOUR_HEADLIGHT_WHITE = hudLed.Color(0, 0, 30, 255);
	uint32_t COLOUR_YELLOW = hudLed.Color(100, 255, 0);
	uint32_t COLOUR_RED = hudLed.Color(255, 0, 0);
	uint32_t COLOUR_DARK_RED = hudLed.Color(100, 0, 0);
	uint32_t COLOUR_GREEN = hudLed.Color(0, 255, 0);

	void setLed(uint32_t color)
	{
		hudLed.setPixelColor(0, color);
		hudLed.show();
	}

	// enum StateID
	// {
	// 	ST_UNKNOWN = 0,
	// 	ST_ONE,
	// };

	// enum Trigger
	// {
	// 	ZONE_UP,
	// 	ZONE_DOWN,
	// };

	// elapsedMillis sinceEntered = 0;
	// elapsedMillis sinceFlashed = 0;
	// uint32_t zoneColour = COLOUR_OFF;

	// void enter_state_zone1()
	// {
	// 	sinceEntered = 0;
	// 	zoneColour = COLOUR_GREY;
	// 	setLed(zoneColour);
	// }

	// void enter_state_zone2()
	// {
	// 	sinceEntered = 0;
	// 	zoneColour = COLOUR_WHITE;
	// 	setLed(zoneColour);
	// }

	// void enter_state_zone3()
	// {
	// 	sinceEntered = 0;
	// 	zoneColour = COLOUR_YELLOW;
	// 	setLed(zoneColour);
	// }

	// void enter_state_zone4()
	// {
	// 	sinceEntered = 0;
	// 	zoneColour = COLOUR_RED;
	// 	setLed(zoneColour);
	// }

	// void enter_state_zone5()
	// {
	// 	sinceEntered = 0;
	// 	zoneColour = COLOUR_DARK_RED;
	// 	setLed(zoneColour);
	// }

	// void on_state()
	// {
	// 	// flash for first 2 beats?
	// }

	// State stateZone1(&enter_state_zone1, &on_state, NULL);
	// State stateZone2(&enter_state_zone2, &on_state, NULL);
	// State stateZone3(&enter_state_zone3, &on_state, NULL);
	// State stateZone4(&enter_state_zone4, &on_state, NULL);
	// State stateZone5(&enter_state_zone5, &on_state, NULL);

	// Fsm fsm(&stateZone2);

	// void addTransitions()
	// {
	// 	fsm.add_transition(&stateZone1, &stateZone2, Trigger::ZONE_UP, NULL);
	// 	fsm.add_transition(&stateZone2, &stateZone3, Trigger::ZONE_UP, NULL);
	// 	fsm.add_transition(&stateZone3, &stateZone4, Trigger::ZONE_UP, NULL);
	// 	fsm.add_transition(&stateZone4, &stateZone5, Trigger::ZONE_UP, NULL);

	// 	fsm.add_transition(&stateZone5, &stateZone4, Trigger::ZONE_DOWN, NULL);
	// 	fsm.add_transition(&stateZone4, &stateZone3, Trigger::ZONE_DOWN, NULL);
	// 	fsm.add_transition(&stateZone3, &stateZone2, Trigger::ZONE_DOWN, NULL);
	// 	fsm.add_transition(&stateZone2, &stateZone1, Trigger::ZONE_DOWN, NULL);
	// }
}