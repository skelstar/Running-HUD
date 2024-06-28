#pragma once
#include <Arduino.h>
#include "BLEDevice.h"

namespace Bluetooth
{
	static BLEUUID hrServiceUUID(BLEUUID((uint16_t)0x180d));			   // BLE Heart Rate Service
	static BLEUUID hrMeasureCharacteristicUUID(BLEUUID((uint16_t)0x2A37)); // BLE Heart Rate Measure Characteristic

	static BLEUUID genericMediaControlServiceUUID(BLEUUID((uint16_t)0x1849));
	static BLEUUID controllerCharacteristicUUID(BLEUUID((uint16_t)0x1234));

	const char *hudControllerAddress = "3c:71:bf:45:fe:16";
	const char *mockHrmAddress = "b4:e6:2d:8b:93:f7";

	static boolean doScan = true;
	static boolean doConnect = false;
	static boolean connected = false;
	static BLEAdvertisedDevice *hrmDevice;
	static BLEAdvertisedDevice *controllerDevice;
	static BLERemoteCharacteristic *pRemoteCharacteristic;

	class ClientInfo
	{
	public:
		BLEClient *client;
		BLEUUID serviceUUID;
		bool doScan;
		bool doConnect;
	} controllerState, hrmState;

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
			// Serial.printf("onResult() \n");
			if (advertisedDevice.haveServiceUUID())
			{
				// Serial.printf("onResult() -> Advertised Service haveServiceUUID \n");
				if (advertisedDevice.isAdvertisingService(hrServiceUUID) && !hrmClient->isConnected())
				{
					std::string name = advertisedDevice.getName();
					Serial.printf("onResult() -> FOUND %s\n", name.c_str());
					hrmDevice = new BLEAdvertisedDevice(advertisedDevice);
					hrmState.doConnect = true;
					hrmState.doScan = false;
				}
				else if (advertisedDevice.isAdvertisingService(genericMediaControlServiceUUID) && !controllerClient->isConnected())
				{
					std::string name = advertisedDevice.getName();
					Serial.printf("onResult() -> FOUND %s\n", name.c_str());
					controllerDevice = new BLEAdvertisedDevice(advertisedDevice);
					controllerState.doConnect = true;
					controllerState.doScan = false;
				}
			}
			if (!hrmState.doScan && !controllerState.doScan)
			{
				Serial.printf("Stopping scan \n");
				BLEDevice::getScan()->stop();
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
		// Serial.printf("Notify callback! \n");
		Serial.printf("Notify callback! %s \n", pBLERemoteCharacteristic->getUUID().toString().c_str());
		// handleHeartRate(pData[1]);
	}

	bool connectToServer(BLEClient *client, BLEAdvertisedDevice *device, BLEUUID serviceUUID, BLEUUID characteristicUUID)
	{
		bool connect = false;

		client->setClientCallbacks(new MyClientCallback());
		connect = client->connect(device);
		Serial.printf("Connecting client (%s): %s \n",
					  device->getName().c_str(),
					  connect ? "OK" : "FAIL");

		// try get service
		BLERemoteService *pRemoteService = client->getService(serviceUUID);
		if (pRemoteService == nullptr)
		{
			Serial.printf("client FAIL to get remote service.. \n");
			client->disconnect();
			return false;
		}

		pRemoteCharacteristic = pRemoteService->getCharacteristic(characteristicUUID);
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

		controllerState.doScan = true;
		hrmState.doScan = true;
	}

	void PerformConnection()
	{
		if (hrmClient->isConnected() == false && hrmState.doConnect)
		{
			Serial.printf("Connecting to HRM \n");
			connectToServer(hrmClient, hrmDevice, hrServiceUUID, hrMeasureCharacteristicUUID);
			hrmState.doConnect = false;
		}

		if (controllerClient->isConnected() == false && controllerState.doConnect)
		{
			Serial.printf("Connecting to HUD Controller \n");
			connectToServer(controllerClient, controllerDevice, genericMediaControlServiceUUID, controllerCharacteristicUUID);
			controllerState.doConnect = false;
		}
	}

	void StartScanning()
	{
		// if (!controllerClient->isConnected() || !hrmClient->isConnected())
		if (controllerState.doScan || hrmState.doScan)
		{
			sendStatus(DISCONNECTED);

			Serial.printf("controllerClient->isConnected(): %s \n", controllerClient->isConnected() ? "YES" : "NO");
			Serial.printf("hrmClient->isConnected(): %s \n", hrmClient->isConnected() ? "YES" : "NO");

			Serial.printf("----------------\n[info] Performing Scan...\n", millis());
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
		// Serial.printf("xBluetoothQueue Send: %s id: %lu\n", getConnectionStatus(status), packet.id);
	}
}