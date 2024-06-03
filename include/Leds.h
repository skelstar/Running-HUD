#include <Fsm.h>
#include <elapsedMillis.h>
#include <Adafruit_NeoPixel.h>

#define PIXEL_COUNT 1
#define BRIGHT_MAX 256

namespace Leds
{
	Adafruit_NeoPixel indicatorLed = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRBW + NEO_KHZ800);

	uint32_t COLOUR_OFF = indicatorLed.Color(0, 0, 0);
	uint32_t COLOUR_RED = indicatorLed.Color(255, 0, 0);
	uint32_t COLOUR_DARK_RED = indicatorLed.Color(100, 0, 0);
	uint32_t COLOUR_GREEN = indicatorLed.Color(0, 255, 0);

	void setindicator(uint32_t color)
	{
		for (uint16_t i = 0; i < indicatorLed.numPixels(); i++)
		{
			indicatorLed.setPixelColor(i, color);
		}
		indicatorLed.show();
	}

	enum StateID
	{
		ST_UNKNOWN = 0,
		ST_ONE,
	};

	enum HrTrigger
	{
		EnteringZ1LOW,
		EnteringZ1HIGH,
		EnteringZ2LOW,
		EnteringZ2HIGH,
		EnteringZ3LOW,
		EnteringZ3HIGH,

	};

	void enter_state_zone1()
	{
		setindicator(COLOUR_GREEN);
	}

	void enter_state_zone2()
	{
		setindicator(COLOUR_RED);
	}

	State stateZone1(&enter_state_zone1, NULL, NULL);
	State stateZone2(&enter_state_zone2, NULL, NULL);

	Fsm fsm(&stateZone2);

	void addTransitions()
	{
		fsm.add_transition(&stateZone1, &stateZone2, HrTrigger::EnteringZ2LOW, NULL);
		fsm.add_transition(&stateZone2, &stateZone1, HrTrigger::EnteringZ1HIGH, NULL);
	}

	void trigger(HrTrigger event)
	{
		fsm.trigger(event);
	}
}