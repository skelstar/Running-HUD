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
	uint32_t COLOUR_GREY = hudLed.Color(0, 0, 0, 100);
	uint32_t COLOUR_WHITE = hudLed.Color(0, 0, 0, 255);
	uint32_t COLOUR_HEADLIGHT_WHITE = hudLed.Color(0, 0, 30, 255);
	uint32_t COLOUR_YELLOW = hudLed.Color(255, 255, 0);
	uint32_t COLOUR_RED = hudLed.Color(255, 0, 0);
	uint32_t COLOUR_DARK_RED = hudLed.Color(100, 0, 0);
	uint32_t COLOUR_GREEN = hudLed.Color(0, 255, 0);
	uint32_t COLOUR_BLUE = hudLed.Color(0, 0, 255);

	enum Brightness
	{
		BRIGHT_LOW,
		BRIGHT_MED,
		BRIGHT_HIGH,
		BRIGHT_HIGHER,
	};

	enum FlashSchema
	{
		FLASHES_IN_SECOND,
		FIFTY_FIFTY,
	};

	struct FlashPattern
	{
		uint8_t flashesPerSecond;
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
			hudLed.setBrightness(1);
			hudLed.show();
			break;
		case BRIGHT_MED:
			_brightness = brightness;
			hudLed.setBrightness(3);
			hudLed.show();
			break;
		case BRIGHT_HIGH:
			_brightness = brightness;
			hudLed.setBrightness(20);
			hudLed.show();
			break;
		case BRIGHT_HIGHER:
			_brightness = brightness;
			hudLed.setBrightness(255);
			hudLed.show();
			break;
		default:
			Serial.printf("Leds::setBrightness() OUT OF RANGE: %d\n", brightness);
			return;
		}
		Serial.printf("Brightness now: %d\n", hudLed.getBrightness());
	}

	void cycleBrightnessUp()
	{
		_brightness++;
		if (_brightness > BRIGHT_HIGHER)
			_brightness = BRIGHT_LOW;
		setBrightness(_brightness);
	}

	void setLed(uint32_t color)
	{
		hudLed.setPixelColor(0, color);
		hudLed.show();
	}

	SimpleFSM fsm;

	enum Trigger
	{
		TR_DISCONNECTED,
		TR_ZONE_1,
		TR_ZONE_2,
		TR_ZONE_3,
		TR_ZONE_4,
		TR_ZONE_5,
	};

	enum StateName
	{
		STATE_DISCONNECTED,
		STATE_ZONE1,
		STATE_ZONE1_UP,
		STATE_ZONE2,
		STATE_ZONE3,
		STATE_ZONE4,
		STATE_ZONE5,
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
		uint32_t colour;
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

	void on_enter_disconnected()
	{
		Serial.printf("on_enter_disconnected()\n");
		ledColour = COLOUR_BLUE;
		setLed(ledColour);
		thisZone.schema = FlashSchema::FIFTY_FIFTY;
	}

	void on_in_disconnected()
	{
		handleSchema(thisZone.schema);
	}

	void on_enter_zone()
	{
		Serial.printf("on_enter_zone() zone %d\n", thisZone.number);
		sinceEntered = 0;
		flashCounter = 0;
	}

	void on_in_zone()
	{
		handleSchema(thisZone.schema);
	}

	// void on_enter_zone_up()
	// {
	// 	Serial.printf("on_enter_zone_up()\n");
	// }

	// void on_in_up_zone()
	// {
	// 	Serial.printf("on_in_up_zone()\n");
	// }

	State zone[] = {
		State("stateDisconnected", &on_enter_disconnected, &on_in_disconnected),
		State("stateZone1", &on_enter_zone, &on_in_zone),
		// State("stateZone1_Up", &on_enter_zone_up, &on_in_up_zone),
		State("stateZone2", &on_enter_zone, &on_in_zone),
		State("stateZone3", &on_enter_zone, &on_in_zone),
		State("stateZone4", &on_enter_zone, &on_in_zone),
		State("stateZone5", &on_enter_zone, &on_in_zone),
	};

	void onInZone1()
	{
		thisZone.number = 1;
		ledColour = COLOUR_GREY;
		setZoneFlashes(1, FIVE_SECONDS);
	}

	void onInZone2()
	{
		thisZone.number = 2;
		ledColour = COLOUR_WHITE;
		setZoneFlashes(1, THREE_SECONDS);
	}

	void onInZone3()
	{
		thisZone.number = 3;
		ledColour = COLOUR_YELLOW;
		setZoneFlashes(3, TWO_SECONDS);
	}

	void onInZone4()
	{
		thisZone.number = 4;
		ledColour = COLOUR_RED;
		setZoneFlashes(2, ONE_SECONDS);
	}

	void onInZone5()
	{
		thisZone.number = 5;
		ledColour = COLOUR_DARK_RED;
		setZoneFlashes(3, ONE_SECONDS);
	}

	// TimedTransition timedTransitions[] = {
	// 	TimedTransition(&zone[STATE_ZONE1_UP], &zone[STATE_ZONE2], 1000),
	// };

	Transition transitions[] = {
		// going UP
		Transition(&zone[STATE_ZONE1], &zone[STATE_ZONE2], Trigger::TR_ZONE_2, &onInZone2),
		Transition(&zone[STATE_ZONE2], &zone[STATE_ZONE3], Trigger::TR_ZONE_3, &onInZone3),
		Transition(&zone[STATE_ZONE3], &zone[STATE_ZONE4], Trigger::TR_ZONE_4, &onInZone4),
		Transition(&zone[STATE_ZONE4], &zone[STATE_ZONE5], Trigger::TR_ZONE_5, &onInZone5),

		// going DOWN
		Transition(&zone[STATE_ZONE5], &zone[STATE_ZONE4], Trigger::TR_ZONE_4, &onInZone4),
		Transition(&zone[STATE_ZONE4], &zone[STATE_ZONE3], Trigger::TR_ZONE_3, &onInZone3),
		Transition(&zone[STATE_ZONE3], &zone[STATE_ZONE2], Trigger::TR_ZONE_2, &onInZone2),
		Transition(&zone[STATE_ZONE2], &zone[STATE_ZONE1], Trigger::TR_ZONE_1, &onInZone1),

		// from disconnected
		Transition(&zone[STATE_DISCONNECTED], &zone[STATE_ZONE1], Trigger::TR_ZONE_1, &onInZone1),
		Transition(&zone[STATE_DISCONNECTED], &zone[STATE_ZONE2], Trigger::TR_ZONE_2, &onInZone1),
		Transition(&zone[STATE_DISCONNECTED], &zone[STATE_ZONE3], Trigger::TR_ZONE_3, &onInZone1),
		Transition(&zone[STATE_DISCONNECTED], &zone[STATE_ZONE4], Trigger::TR_ZONE_4, &onInZone1),
		Transition(&zone[STATE_DISCONNECTED], &zone[STATE_ZONE5], Trigger::TR_ZONE_5, &onInZone1),

		// to disconnected
		Transition(&zone[STATE_ZONE1], &zone[STATE_DISCONNECTED], Trigger::TR_DISCONNECTED),
		Transition(&zone[STATE_ZONE2], &zone[STATE_DISCONNECTED], Trigger::TR_DISCONNECTED),
		Transition(&zone[STATE_ZONE3], &zone[STATE_DISCONNECTED], Trigger::TR_DISCONNECTED),
		Transition(&zone[STATE_ZONE4], &zone[STATE_DISCONNECTED], Trigger::TR_DISCONNECTED),
		Transition(&zone[STATE_ZONE5], &zone[STATE_DISCONNECTED], Trigger::TR_DISCONNECTED),
	};

	void SetupFsm()
	{
		int num_transitions = sizeof(transitions) / sizeof(Transition);
		fsm.add(transitions, num_transitions);

		ledColour = COLOUR_BLUE;
		fsm.setInitialState(&zone[STATE_DISCONNECTED]);
	}
}