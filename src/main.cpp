#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "Leds.h"
#include "ZonesStateMachine.h"
#include "ButtonMain.h"
#include "ButtonAcc.h"

static void handleHeartRate(uint8_t hr);

TaskHandle_t bluetoothTaskHandle = NULL;

#include "Bluetooth.h"
#include "Tasks/BluetoothTask.h"

void setup()
{
	Serial.begin(115200);

	// Leds::hudLed.begin();
	// Leds::hudLed.show(); // Initialize all pixels to 'off'

	// Leds::setBrightness(Leds::BRIGHT_MED);
	// Leds::setLed(Leds::COLOUR_BLUE);

	pinMode(M5_LED_PIN, OUTPUT);

	// Leds::SetupFsm();

	// ZonesStateMachine::SetupFsm();

	// ButtonMain::initialise();
	// ButtonAcc::initialise();

	// vTaskDelay(200);
	xTaskCreatePinnedToCore(
		BluetoothTask::task1,
		"BluetoothTask1",
		/*stack depth*/ 4096,
		/*params*/ NULL,
		/*priority*/ 1,
		&bluetoothTaskHandle,
		/*core*/ 1);

	// vTaskStartScheduler();

	// BLE setup
	// Bluetooth::initialise();
}

int loopNum = 0;
elapsedMillis sinceFlashedLed = 0;
bool m5ledState = false;

void loop()
{
	// Bluetooth::PerformConnection();

	// ButtonMain::button.loop();
	// ButtonAcc::button.loop();

	// ZonesStateMachine::fsm.run(100);
	// Leds::fsm.run(100);

	if (sinceFlashedLed > 500)
	{
		sinceFlashedLed = 0;
		m5ledState = !m5ledState;
		digitalWrite(M5_LED_PIN, m5ledState);
	}

	// Bluetooth::PerformScan();

	vTaskDelay(10);
}
