#include <Arduino.h>

#include "Leds.h"
#include "ZonesStateMachine.h"
#include "ButtonMain.h"
#include "ButtonAcc.h"

static void handleHeartRate(uint8_t hr);

#include "Bluetooth.h"

void setup()
{
	Serial.begin(115200);

	Leds::indicatorLed.begin();
	Leds::indicatorLed.show(); // Initialize all pixels to 'off'

	pinMode(M5_LED_PIN, OUTPUT);

	// Leds::addTransitions();

	ZonesStateMachine::SetupFsm();

	ButtonMain::initialise();
	ButtonAcc::initialise();

	// BLE setup
	Bluetooth::initialise();
}

int loopNum = 0;
elapsedMillis sinceFlashedLed = 0;
bool m5ledState = false;

void loop()
{
	Bluetooth::PerformConnection();

	// Leds::fsm.run_machine();
	ButtonMain::button.loop();
	ButtonAcc::button.loop();

	ZonesStateMachine::fsm.run(200);

	if (sinceFlashedLed > 500)
	{
		sinceFlashedLed = 0;
		m5ledState = !m5ledState;
		digitalWrite(M5_LED_PIN, m5ledState);
	}

	Bluetooth::PerformScan();
}
