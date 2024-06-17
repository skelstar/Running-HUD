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
}
