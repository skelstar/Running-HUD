#pragma once
#include <SimpleFsm.h>
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
	uint32_t COLOUR_BLUE = hudLed.Color(0, 0, 255);

	enum Brightness
	{
		BRIGHT_LOW,
		BRIGHT_MED,
		BRIGHT_HIGH
	};

	uint16_t flashingRateMs = 200;
	bool flashingState = false;
	uint8_t _brightness = BRIGHT_MED; // middle

	void setBrightness(uint8_t brightness)
	{
		switch (brightness)
		{
		case BRIGHT_LOW:
			_brightness = brightness;
			hudLed.setBrightness(5);
			hudLed.show();
			break;
		case BRIGHT_MED:
			_brightness = brightness;
			hudLed.setBrightness(30);
			hudLed.show();
			break;
		case BRIGHT_HIGH:
			_brightness = brightness;
			hudLed.setBrightness(200);
			hudLed.show();
			break;
		}
	}

	void increaseBrightness()
	{
		_brightness = _brightness != BRIGHT_LOW ? _brightness - 1 : _brightness;
		setBrightness(_brightness);
	}

	void decreaseBrightness()
	{
		_brightness = _brightness != BRIGHT_HIGH ? _brightness + 1 : _brightness;
		setBrightness(_brightness);
	}

	void setLed(uint32_t color)
	{
		hudLed.setPixelColor(0, color);
		hudLed.show();
	}

	SimpleFSM fsm;

	enum StateID
	{
		// ST_UNKNOWN = 0,
		// ST_ONE,
	};

	enum Trigger
	{
		START_FLASHING,
		SOLID,
	};

	enum StateName
	{
		STATE_FLASHING,
		STATE_SOLID,
	};

	elapsedMillis sinceEntered = 0;
	elapsedMillis sinceFlashed = 0;
	uint32_t ledColour = Leds::COLOUR_OFF;

	void on_enter_flashing()
	{
		flashingState = false;
		setLed(ledColour);
	}

	void on_enter_solid()
	{
		setLed(ledColour);
	}

	void on_state_flashing()
	{
		if (sinceEntered > flashingRateMs)
		{
			flashingState = !flashingState;
			setLed(flashingState ? ledColour : COLOUR_OFF);
		}
	}

	State zone[] = {
		State("stateFlashing", &on_enter_flashing, &on_state_flashing),
		State("stateSolid", &on_enter_solid),
	};

	void onRun()
	{
		sinceEntered = 0;
	}

	bool onGuard()
	{
		return true;
	}

	Transition transitions[] = {
		Transition(&zone[STATE_FLASHING], &zone[STATE_SOLID], Trigger::SOLID, &onRun, "", &onGuard),
		Transition(&zone[STATE_SOLID], &zone[STATE_FLASHING], Trigger::START_FLASHING, &onRun, "", &onGuard),

		Transition(&zone[STATE_SOLID], &zone[STATE_SOLID], Trigger::SOLID, &onRun, "", &onGuard),
	};

	void SetupFsm()
	{
		int num_transitions = sizeof(transitions) / sizeof(Transition);
		fsm.add(transitions, num_transitions);

		fsm.setInitialState(&zone[STATE_SOLID]);
	}
}