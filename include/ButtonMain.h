#include <Button2.h>

namespace ButtonMain
{
#define LONGCLICK_MS 1000

	Button2 button;

	void clickHandler(Button2 &btn)
	{
		Serial.printf("Main Button clicked\n");
	}

	void longClickDetectedHandler(Button2 &btn)
	{
		Serial.printf("Main Button long click\n");
		Leds::increaseBrightness();
	}

	void initialise()
	{
		button.begin(MAIN_BUTTON_PIN);
		button.setLongClickTime(LONGCLICK_MS);

		// handlers
		button.setClickHandler(&clickHandler);
		button.setLongClickDetectedHandler(&longClickDetectedHandler);
	}
}