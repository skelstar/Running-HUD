#pragma once
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

namespace Bluetooth
{

	static boolean doScan = true;
	static boolean doConnect = false;
	static boolean connected = false;
	static BLEAdvertisedDevice *hrmDevice;
	static BLERemoteCharacteristic *pRemoteCharacteristic;

	// prototypes
	void sendStatus(ConnectionStatus status);
	void subscribeToNotifications(BLEClient *client);

	static BLEUUID hrmServiceUUID(BLEUUID((uint16_t)0x180d)); // BLE Heart Rate Service
	static BLEUUID hrCharUUID(BLEUUID((uint16_t)0x2A37));	  // BLE Heart Rate Measure Characteristic

	const char *hudControllerAddress = "3c:71:bf:45:fe:16";
	const char *mockHrmAddress = "b4:e6:2d:8b:93:f7";

	BLEClient *pClient1;
	BLEClient *pClient2;

	class Packet
	{
	public:
		uint8_t status;
		uint8_t hr;
		unsigned long id = -1;
	} packet;

	static void handleHeartRate(uint8_t hr)
	{
		Serial.printf("My HR is %dbpm\n", hr);
		packet.id++;
		packet.hr = hr;
		Packet *data;
		data = &packet;

		xQueueSend(xBluetoothQueue, (void *)&data, TICKS_5ms);
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

	class MyClientCallback : public BLEClientCallbacks
	{
		void onConnect(BLEClient *pclient)
		{
			Serial.printf("%lu [info] Connected: %s\n", millis(), pclient->getPeerAddress().toString().c_str());

			sendStatus(CONNECTED);
		}

		void onDisconnect(BLEClient *pclient)
		{
			connected = false;
			doScan = true;
			Serial.printf("%lu [info] Disconnected: %s\n", millis(), pclient->getPeerAddress().toString().c_str());

			sendStatus(DISCONNECTED);
		}
	};

	class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
	{
		void onResult(BLEAdvertisedDevice advertisedDevice)
		{
			std::string address = advertisedDevice.getAddress().toString();
			std::string name = advertisedDevice.getName();
			Serial.printf("server found: %s %s \n",
						  !name.empty() ? name.c_str() : "Unknown",
						  address.c_str());

			if (address == hudControllerAddress && !pClient1->isConnected())
			{
				Serial.printf("Found HUD Controller, connecting...\n");
				bool connected = pClient1->connect(&advertisedDevice);
				subscribeToNotifications(pClient1);
				Serial.printf("Connection: %s \n", connected ? "YES" : "FAIL");
			}
			if (address == mockHrmAddress && !pClient2->isConnected())
			{
				// Serial.printf("Found %s, connecting...\n", name.c_str());
				Serial.printf("Found HRM Mock, connecting...\n");
				pClient2->connect(&advertisedDevice);
				subscribeToNotifications(pClient2);
				// Serial.printf("Connection: %s \n", connected ? "YES" : "FAIL");
			}
		}
	};

	void subscribeToNotifications(BLEClient *client)
	{
		BLERemoteService *pRemoteService = client->getService(hrmServiceUUID);
		if (pRemoteService == nullptr)
		{
			Serial.println("Failed to find service.");
			return;
		}

		BLERemoteCharacteristic *pRemoteCharacteristic = pRemoteService->getCharacteristic(hrCharUUID);
		if (pRemoteCharacteristic == nullptr)
		{
			Serial.println("Failed to find characteristic.");
			return;
		}

		if (pRemoteCharacteristic->canNotify())
		{
			pRemoteCharacteristic->registerForNotify(notifyCallback);
			Serial.println("Subscribed to notifications.");
		}
	}

#define SECONDS_30 30

	void handleReconnections()
	{
		// Handle reconnections
		if (!pClient1->isConnected())
		{
			Serial.println("Reconnecting to server 1...");
			BLEDevice::getScan()->start(SECONDS_30, false);
		}

		if (!pClient2->isConnected())
		{
			Serial.println("Reconnecting to server 2...");
			BLEDevice::getScan()->start(SECONDS_30, false);
		}

		delay(1000); // Adjust delay as needed
	}

	elapsedMillis sinceStartedScanning;

	void startScan()
	{
		Serial.printf("Starting scan ......... \n");
		BLEScan *pBLEScan = BLEDevice::getScan();
		pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
		pBLEScan->setActiveScan(true); // Active scan uses more power but gets results faster
		pBLEScan->start(SECONDS_30, false);
		sinceStartedScanning = 0;
	}

	void initialise()
	{
		BLEDevice::init("");
		BLEScan *pBLEScan = BLEDevice::getScan();

		pClient1 = BLEDevice::createClient();
		pClient2 = BLEDevice::createClient();

		pClient1->setClientCallbacks(new MyClientCallback());
		pClient2->setClientCallbacks(new MyClientCallback());

		sendStatus(DISCONNECTED);

		startScan();
	}

	void sendStatus(ConnectionStatus status)
	{
		packet.id++;
		packet.status = status;
		Packet *data;
		data = &packet;

		xQueueSend(xBluetoothQueue, (void *)&data, (TickType_t)1);
		Serial.printf("xBluetoothQueue Send: %s id: %lu\n", getConnectionStatus(status), packet.id);
	}
}