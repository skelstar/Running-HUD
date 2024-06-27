#pragma once
#include <Arduino.h>
#include "BLEDevice.h"

namespace Bluetooth
{
	static BLEUUID hrServiceUUID(BLEUUID((uint16_t)0x180d));			   // BLE Heart Rate Service
	static BLEUUID hrMeasureCharacteristicUUID(BLEUUID((uint16_t)0x2A37)); // BLE Heart Rate Measure Characteristic

	const char *hudControllerAddress = "3c:71:bf:45:fe:16";
	const char *mockHrmAddress = "b4:e6:2d:8b:93:f7";

	static boolean doScan = true;
	static boolean doConnect = false;
	static boolean connected = false;
	static BLEAdvertisedDevice *hrmDevice;
	static BLERemoteCharacteristic *pRemoteCharacteristic;

	BLEClient *controllerClient;
	BLEClient *hrmClient;

	// prototypes
	void sendStatus(ConnectionStatus status);

	class Packet
	{
	public:
		uint8_t status;
		uint8_t hr;
		unsigned long id = -1;
	} packet;

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
			if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(hrServiceUUID))
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

	bool connectToServer(BLEClient *client, const char* address)
	{
		// BLEClient *client1 = BLEDevice::createClient();

		bool connect = false;

		client->setClientCallbacks(new MyClientCallback());
		connect = client->connect(hrmDevice);
		Serial.printf("Connecting client: %s \n", connect ? "OK" : "FAIL");

		// try get service
		BLERemoteService *pRemoteService = client->getService(hrServiceUUID);
		if (pRemoteService == nullptr)
		{
			Serial.printf("client FAIL to get remote service.. \n");
			client->disconnect();
			return false;
		}

		pRemoteCharacteristic = pRemoteService->getCharacteristic(hrMeasureCharacteristicUUID);
		if (pRemoteCharacteristic == nullptr)
		{
			client->disconnect();
			return false;
		}

		if (pRemoteCharacteristic->canNotify())
		{
			pRemoteCharacteristic->registerForNotify(notifyCallback);
		}
		else
		{
			Serial.printf("Remote characteristic can't notify! Client disconnecting \n");
			client->disconnect();
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

		controllerClient = BLEDevice::createClient();
		hrmClient = BLEDevice::createClient();
	}

	// uint8_t numDevicesConnected = 0;

	void PerformConnection()
	{
		if (doConnect)
		{
			bool deviceConnected = false;
			if (hrmClient->isConnected() == false)
				connectToServer(hrmClient);
			else if (controllerClient->isConnected() == false)
				connectToServer(controllerClient);

			if (!deviceConnected)
			{
				Serial.printf("%lu [error] Failed to connect to HRM.\n", millis());
			}
			doConnect = !controllerClient->isConnected() || !hrmClient->isConnected();
		}
	}

	void PerformScan()
	{
		if (!controllerClient->isConnected() || !hrmClient->isConnected())
		{
			sendStatus(DISCONNECTED);

			Serial.printf("controllerClient->isConnected(): %s \n", controllerClient->isConnected() ? "YES" : "NO");
			Serial.printf("hrmClient->isConnected(): %s \n", hrmClient->isConnected() ? "YES" : "NO");

			Serial.printf("[info] Performing Scan...\n", millis());
			BLEDevice::getScan()->start(5, false);
			delay(5000);
		}
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