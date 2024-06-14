#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <elapsedMillis.h>
#include <Types.h>

#include <M5StickC.h>

#include <I2C_MPU6886.h>

namespace AccelerometerTask
{
    TaskHandle_t taskHandle = NULL;
    const char *taskName = "AccelerometerTask";

    const float tapThreshold = 1.6;
    const unsigned long doubleTapWindowL = 200;
    const unsigned long doubleTapWindowH = 300;

    // prototypes
    bool isDoubleTap(elapsedMillis sinceLastTap);
    void sendPacket(ButtonPacket *packet);

    void task1(void *pvParameters)
    {
        elapsedMillis sinceLastTap = 0;

        ButtonPacket packet;

        Serial.printf("%s: Started\n", taskName);

        vTaskDelay(100);

        M5.begin();
        M5.Imu.Init();

        while (1)
        {
            float ax, ay, az;

            M5.IMU.getAccelData(&ax, &ay, &az);

            if (ax > tapThreshold || ay > tapThreshold || az > tapThreshold)
            {
                if (isDoubleTap(sinceLastTap))
                {
                    Serial.printf("Double Tap!\n");

                    packet.id++;
                    packet.button = ButtonOption::ACCEL;
                    packet.event = ButtonEvent::DOUBLE_TAP;

                    sendPacket(&packet);
                }
                sinceLastTap = 0;
            }

            vTaskDelay(TICKS_5ms);
        }
    }

    bool isDoubleTap(elapsedMillis sinceLastTap)
    {
        unsigned long lastTap = (unsigned long)sinceLastTap;
        return lastTap > doubleTapWindowL && lastTap < doubleTapWindowH;
    }

    void sendPacket(ButtonPacket *packet)
    {
        ButtonPacket *data;
        data = packet;
        xQueueSend(xButtonQueue, (void *)&data, TICKS_10ms);
    }
}
