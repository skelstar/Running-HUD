#pragma once
#include <Arduino.h>
#include "BLEDevice.h"

namespace Bluetooth
{
	static BLEUUID serviceUUID(BLEUUID((uint16_t)0x180d)); // BLE Heart Rate Service
	static BLEUUID hrCharUUID(BLEUUID((uint16_t)0x2A37));  // BLE Heart Rate Measure Characteristic

	static boolean doScan = true;
	static boolean doConnect = false;
	static boolean connected = false;
	static BLEAdvertisedDevice *hrmDevice;
	static BLERemoteCharacteristic *pRemoteCharacteristic;

	class MyClientCallback : public BLEClientCallbacks
	{
		void onConnect(BLEClient *pclient)
		{
			Serial.printf("%lu [info] Connected: %s\n", millis(), pclient->getPeerAddress().toString().c_str());
		}

		void onDisconnect(BLEClient *pclient)
		{
			connected = false;
			doScan = true;
			Serial.printf("%lu [info] Disconnected: %s\n", millis(), pclient->getPeerAddress().toString().c_str());
		}
	};

	class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
	{
		void onResult(BLEAdvertisedDevice advertisedDevice)
		{
			if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID))
			{
				Serial.printf("%lu [info] %s\n", millis(), advertisedDevice.toString().c_str());
				BLEDevice::getScan()->stop();
				hrmDevice = new BLEAdvertisedDevice(advertisedDevice);
				doConnect = true;
				doScan = false;
			}
		}
	};

	static void handleHeartRate(uint8_t hr)
	{
		Serial.printf("My HR is %dbpm\n", hr);

		if (hr < 63)
		{
			// Leds::trigger(Leds::HrTrigger::EnteringZ1HIGH);
		}
		else if (hr >= 63)
		{
			// Leds::trigger(Leds::HrTrigger::EnteringZ2LOW);
		}
	}

	// BLE Heart Rate Measure Callback
	static void notifyCallback(
		BLERemoteCharacteristic *pBLERemoteCharacteristic,
		uint8_t *pData,
		size_t length,
		bool isNotify)
	{
		handleHeartRate(pData[1]);
	}

	bool connectToServer()
	{
		BLEClient *pClient = BLEDevice::createClient();
		pClient->setClientCallbacks(new MyClientCallback());
		pClient->connect(hrmDevice);

		BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
		if (pRemoteService == nullptr)
		{
			pClient->disconnect();
			return false;
		}

		pRemoteCharacteristic = pRemoteService->getCharacteristic(hrCharUUID);
		if (pRemoteCharacteristic == nullptr)
		{
			pClient->disconnect();
			return false;
		}

		if (pRemoteCharacteristic->canNotify())
		{
			pRemoteCharacteristic->registerForNotify(notifyCallback);
		}
		else
		{
			pClient->disconnect();
			return false;
		}

		connected = true;
		return true;
	}

	void initialise()
	{
		BLEDevice::init("");
		BLEScan *pBLEScan = BLEDevice::getScan();
		pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
		pBLEScan->setInterval(1349);
		pBLEScan->setWindow(449);
		pBLEScan->setActiveScan(true);
	}

	void PerformConnection()
	{
		if (doConnect)
		{
			if (!connectToServer())
			{
				Serial.printf("%lu [error] Failed to connect to HRM.\n", millis());
			}
			doConnect = false;
		}
	}

	void PerformScan()
	{
		if (doScan)
		{
			Serial.printf("%lu [info] Performing Scan...\n", millis());
			BLEDevice::getScan()->start(5, false);
			delay(5000);
		}
	}
}