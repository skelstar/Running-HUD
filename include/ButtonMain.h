#pragma once
#include <Button2.h>

namespace ButtonMain
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
		// Serial.printf("Main Button clicked\n");
		packet.id++;
		packet.button = ButtonOption::MAIN_BTN;
		packet.event = CLICK;

		sendPacket(&packet);
	}

	void longClickDetectedHandler(Button2 &btn)
	{
		// Serial.printf("Main Button long click\n");

		packet.id++;
		packet.button = ButtonOption::MAIN_BTN;
		packet.event = LONGCLICK;

		sendPacket(&packet);
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