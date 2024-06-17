#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "Types.h"

// Queues
QueueHandle_t xBluetoothQueue;
QueueHandle_t xButtonQueue;

#include "Tasks/BluetoothTask.h"
#include "Tasks/LedsTask.h"
#include "Tasks/ButtonsTask.h"
#include "Tasks/RedLedTask.h"
#include "Tasks/AccelerometerTask.h"
#include "Tasks/DisplayTask.h"

void setup()
{
	Serial.begin(115200);

	xBluetoothQueue = xQueueCreate(1, sizeof(Bluetooth::Packet *));
	xButtonQueue = xQueueCreate(1, sizeof(ButtonPacket *));

	xTaskCreatePinnedToCore(
		BluetoothTask::task1,
		BluetoothTask::taskName,
		/*stack depth*/ 4096,
		/*params*/ NULL,
		/*priority*/ 1,
		&BluetoothTask::taskHandle,
		/*core*/ 1);

	xTaskCreatePinnedToCore(
		Leds::task1,
		"LedsTask",
		/*stack depth*/ 8000,
		/*params*/ NULL,
		/*priority*/ 1,
		&Leds::taskHandle,
		/*core*/ 1);

	xTaskCreatePinnedToCore(
		ButtonsTask::task1,
		ButtonsTask::taskName,
		/*stack depth*/ 2048,
		/*params*/ NULL,
		/*priority*/ 1,
		&ButtonsTask::taskHandle,
		/*core*/ 1);

	xTaskCreatePinnedToCore(
		RedLedTask::task1,
		RedLedTask::taskName,
		/*stack depth*/ 2048,
		/*params*/ NULL,
		/*priority*/ 1,
		&RedLedTask::taskHandle,
		/*core*/ 1);

	DisplayTask::createTask();

	// xTaskCreatePinnedToCore(
	// 	AccelerometerTask::task1,
	// 	AccelerometerTask::taskName,
	// 	/*stack depth*/ 2048,
	// 	/*params*/ NULL,
	// 	/*priority*/ 1,
	// 	&AccelerometerTask::taskHandle,
	// 	/*core*/ 1);
}

int loopNum = 0;

void loop()
{
	vTaskDelay(10);
}
