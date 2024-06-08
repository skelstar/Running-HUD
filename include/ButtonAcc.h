#include <Button2.h>

namespace ButtonAcc
{
#define LONGCLICK_MS 1000

	Button2 button;

	void clickHandler(Button2 &btn)
	{
		Serial.printf("Acc Button clicked\n");
		ZonesStateMachine::fsm.trigger(ZonesStateMachine::Trigger::ZONE_DOWN);
	}

	void longClickDetectedHandler(Button2 &btn)
	{
		// Serial.printf("Acc Button long click\n");
		Leds::decreaseBrightness();
	}

	void initialise()
	{
		button.begin(ACC_BUTTON_PIN);
		button.setLongClickTime(LONGCLICK_MS);

		// handlers
		button.setClickHandler(&clickHandler);
		button.setLongClickDetectedHandler(&longClickDetectedHandler);
	}
}