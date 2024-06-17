#pragma once
#include <Button2.h>

namespace ButtonAcc
{
#define LONGCLICK_MS 1000

	Button2 button;

	ButtonPacket packet;

	void sendPacket(ButtonPacket *packet)
	{
		ButtonPacket *data;
		data = packet;
		xQueueSend(xButtonQueue, (void *)&data, TICKS_10ms);
	}

	void clickHandler(Button2 &btn)
	{
		Serial.printf("Acc Button clicked\n");

		packet.id++;
		packet.button = ButtonOption::ACC_BTN;
		packet.event = ButtonEvent::CLICK;

		sendPacket(&packet);
	}

	void longClickDetectedHandler(Button2 &btn)
	{
		Serial.printf("Acc Button long click\n");

		packet.id++;
		packet.button = ButtonOption::ACC_BTN;
		packet.event = ButtonEvent::LONGCLICK;

		sendPacket(&packet);
	}

	void doubleTapHandler_cb(Button2 &btn)
	{
		Serial.printf("Acc Button double click\n");

		packet.id++;
		packet.button = ButtonOption::ACC_BTN;
		packet.event = ButtonEvent::DOUBLE_TAP;

		sendPacket(&packet);
	}

	void initialise()
	{
		button.begin(ACC_BUTTON_PIN);
		button.setLongClickTime(LONGCLICK_MS);
		button.setDoubleClickTime(500); // default: BTN_DOUBLECLICK_MS = 300

		// handlers
		button.setClickHandler(&clickHandler);
		button.setLongClickDetectedHandler(&longClickDetectedHandler);
		button.setDoubleClickHandler(&doubleTapHandler_cb);
	}
}