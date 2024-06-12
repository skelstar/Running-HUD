#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "Types.h"

#include "Leds.h"
#include "ButtonMain.h"
#include "ButtonAcc.h"

static void handleHeartRate(uint8_t hr);

// Queues
QueueHandle_t xBluetoothQueue;

TaskHandle_t bluetoothTaskHandle = NULL;
TaskHandle_t ledsTaskHandle = NULL;
TaskHandle_t zonesTaskHandle = NULL;

#include "Bluetooth.h"
#include "Tasks/BluetoothTask.h"
#include "Tasks/LedsTask.h"

void setup()
{
	Serial.begin(115200);

	xBluetoothQueue = xQueueCreate(1, sizeof(Bluetooth::Packet *));

	pinMode(M5_LED_PIN, OUTPUT);

	// ButtonMain::initialise();
	// ButtonAcc::initialise();

	xTaskCreatePinnedToCore(
		BluetoothTask::task1,
		"BluetoothTask",
		/*stack depth*/ 4096,
		/*params*/ NULL,
		/*priority*/ 1,
		&bluetoothTaskHandle,
		/*core*/ 1);

	xTaskCreatePinnedToCore(
		LedsTask::task1,
		"LedsTask",
		/*stack depth*/ 8000,
		/*params*/ NULL,
		/*priority*/ 1,
		&ledsTaskHandle,
		/*core*/ 1);
}

int loopNum = 0;
elapsedMillis sinceFlashedLed = 0;
bool m5ledState = false;

void loop()
{
	// ButtonMain::button.loop();
	// ButtonAcc::button.loop();

	if (sinceFlashedLed > 500)
	{
		sinceFlashedLed = 0;
		m5ledState = !m5ledState;
		digitalWrite(M5_LED_PIN, m5ledState);
	}

	// Bluetooth::PerformScan();

	vTaskDelay(10);
}
