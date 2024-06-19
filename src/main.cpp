#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "Types.h"

// Queues
QueueHandle_t xBluetoothQueue;
QueueHandle_t xInputsQueue;

#include "Tasks/BluetoothTask.h"
#include "Tasks/LedsTask.h"
#include "Tasks/ButtonsTask.h"
#include "Tasks/RedLedTask.h"
#include "Tasks/AccelerometerTask.h"
#include "Tasks/DisplayTask.h"
#include "Tasks/ClipDetectTask.h"
#include "Tasks/BuzzerTask.h"

void setup()
{
	Serial.begin(115200);

	xBluetoothQueue = xQueueCreate(1, sizeof(Bluetooth::Packet *));
	xInputsQueue = xQueueCreate(1, sizeof(InputPacket *));

	BluetoothTask::createTask(4096);
	Leds::createTask(8000);
	ButtonsTask::createTask(2048);
	// RedLedTask::createTask(2048);
	// DisplayTask::createTask(2048);
	// BuzzerTask::createTask(2048);
	// ClipDetectTask::createTask(1024);
	// AccelerometerTask::createTask(2048);
}

void loop()
{
	vTaskDelay(TICKS_10ms);
}
