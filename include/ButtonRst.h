#pragma once
#include <M5StickC.h>
#include "Types.h"

namespace ButtonRst
{
#define LONGCLICK_MS 1000

	ButtonPacket packet;

	void sendPacket(ButtonPacket *packet)
	{
		ButtonPacket *data;
		data = packet;
		xQueueSend(xButtonQueue, (void *)&data, TICKS_10ms);
	}

	void clickHandler()
	{
		Serial.printf("Rst Button clicked\n");

		packet.id++;
		packet.button = ButtonOption::RST_BTN;
		packet.event = ButtonEvent::CLICK;

		sendPacket(&packet);

		/*
		// close tft voltage output.
		M5.Axp.SetLDO2(false);
		// close tft lcd voltage output
		M5.Axp.SetLDO3(false);
		*/
	}

	void longClickDetectedHandler()
	{
		Serial.printf("Rst Button long click\n");

		packet.id++;
		packet.button = ButtonOption::RST_BTN;
		packet.event = ButtonEvent::LONGCLICK;

		sendPacket(&packet);
	}

	void loop()
	{
		switch (M5.Axp.GetBtnPress())
		{
		case 0x02:
			clickHandler();
			break;
		case 0x01:
			longClickDetectedHandler();
			break;
		}
	}

	void initialise()
	{
		M5.begin();
		M5.Axp.begin();
	}
}