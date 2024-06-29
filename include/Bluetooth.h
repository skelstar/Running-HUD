#pragma once
#include <Arduino.h>
#include "BLEDevice.h"

namespace Bluetooth
{
	static BLEUUID hrServiceUUID(BLEUUID((uint16_t)0x180d));			   // BLE Heart Rate Service
	static BLEUUID hrMeasureCharacteristicUUID(BLEUUID((uint16_t)0x2A37)); // BLE Heart Rate Measure Characteristic

	static BLEUUID genericMediaControlServiceUUID(BLEUUID((uint16_t)0x1849));
	static BLEUUID controllerCharacteristicUUID(BLEUUID((uint16_t)0x1234));

	// const char *hudControllerAddress = "3c:71:bf:45:fe:16";
	// const char *mockHrmAddress = "b4:e6:2d:8b:93:f7";

	static BLEAdvertisedDevice *hrmDevice;
	static BLEAdvertisedDevice *controllerDevice;
	static BLERemoteCharacteristic *pRemoteCharacteristic;

#define NO_SCAN_TIMEOUT 0
#define REPORT 1
#define DONT_REPORT 0

	class ClientInfo
	{
	public:
		BLEClient *client;
		BLEUUID serviceUUID;
		bool doConnect;

		ClientInfo(const char *name, unsigned long scanTimeout = NO_SCAN_TIMEOUT, bool report = DONT_REPORT)
		{
			_name = name;
			_scanTimeout = scanTimeout;
			client = BLEDevice::createClient();
			_report = report;
		}

		// if less than _scanTimeout or _scanTimeout is zero (ie no timeout)
		bool doScan(bool report = DONT_REPORT)
		{
			bool doTheScan = (_scanTimeout == NO_SCAN_TIMEOUT || millis() < _scanTimeout) && _doScan;
			// if (report)
			// 	Serial.printf("doTheScan: %d timeout: %lu millis: %lu doScan: %d \n",
			// 				  doTheScan, _scanTimeout, millis(), _doScan);
			return doTheScan;
		}

		void disconnected()
		{
			_doScan = true;
			Serial.printf("[event] %s disconnected! \n", _name);
		}

		void found()
		{
			_doScan = false;
			doConnect = true;
			Serial.printf("[event] %s found! \n", _name);
		}

	private:
		const char *_name = "NO NAME";
		bool _doScan = true;
		unsigned long _scanTimeout;
		bool _report;
	};

	ClientInfo ctrlr("Controller", ONE_MINUTE, DONT_REPORT);
	ClientInfo heart("HeartRate", NO_SCAN_TIMEOUT, DONT_REPORT);

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
			Serial.printf("[info] Connected: %s\n", pclient->getPeerAddress().toString().c_str());

			sendStatus(CONNECTED);
		}

		void onDisconnect(BLEClient *pclient)
		{
			Serial.printf("[info] Disconnected: %s\n", pclient->getPeerAddress().toString().c_str());

			std::string disconnectedBleDeviceUUID = pclient->getPeerAddress().toString();
			if (disconnectedBleDeviceUUID == hrmDevice->getAddress().toString())
			{
				heart.disconnected();
			}
			else if (disconnectedBleDeviceUUID == controllerDevice->getAddress().toString())
			{
				ctrlr.disconnected();
			}
			else
			{
				Serial.printf("[info] Unknown device disconnected: %s\n", pclient->getPeerAddress().toString().c_str());
			}

			sendStatus(DISCONNECTED);
		}
	};

	class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
	{
		void onResult(BLEAdvertisedDevice advertisedDevice)
		{
			if (advertisedDevice.haveServiceUUID())
			{
				if (advertisedDevice.isAdvertisingService(hrServiceUUID) &&
					!heart.client->isConnected())
				{
					std::string name = advertisedDevice.getName();
					Serial.printf("onResult() -> FOUND %s\n", name.c_str());
					// TODO make device a part of state?
					hrmDevice = new BLEAdvertisedDevice(advertisedDevice);
					heart.found();
				}
				else if (advertisedDevice.isAdvertisingService(genericMediaControlServiceUUID) &&
						 !ctrlr.client->isConnected())
				{
					std::string name = advertisedDevice.getName();
					Serial.printf("onResult() -> FOUND %s\n", name.c_str());
					controllerDevice = new BLEAdvertisedDevice(advertisedDevice);
					ctrlr.found();
				}
			}

			// if we have found both device, then we can stop scanning
			if (!heart.doScan() && !ctrlr.doScan())
			{
				Serial.printf("Stopping scan \n");
				BLEDevice::getScan()->stop();
			}
		}
	};

	void handleHudControllerAction(uint8_t action)
	{
		Serial.printf("[Notify] Action is %s \n", getControllerAction(action));
		InputPacket *data;
		data->id = millis();
		data->input = CONTROLLER_BTN;
		data->event = action;
		xQueueSend(xInputsQueue, (void *)&data, TICKS_10ms);
	}

	static void handleHeartRate(uint8_t hr)
	{
		Serial.printf("[Notify] HR is %dbpm\n", hr);
		packet.id++;
		packet.hr = hr;
		Packet *data;
		data = &packet;

		xQueueSend(xBluetoothQueue, (void *)&data, TICKS_5ms);
	}

	static void notifyCallback(
		BLERemoteCharacteristic *pBLERemoteCharacteristic,
		uint8_t *pData,
		size_t length,
		bool isNotify)
	{
		std::string notifyCharUUID = pBLERemoteCharacteristic->getUUID().toString();

		if (notifyCharUUID == hrMeasureCharacteristicUUID.toString())
		{
			handleHeartRate(pData[1]);
		}
		else if (notifyCharUUID == controllerCharacteristicUUID.toString())
		{
			handleHudControllerAction(pData[0]);
			// Serial.printf("Notify callback! %s \n", "Controller Characteristic");
		}
	}

	bool connectToServer(ClientInfo clientInfo, BLEAdvertisedDevice *device, BLEUUID serviceUUID, BLEUUID characteristicUUID)
	{
		bool connect = false;

		clientInfo.client->setClientCallbacks(new MyClientCallback());
		connect = clientInfo.client->connect(device);
		Serial.printf("Connecting client (%s): %s \n", device->getName().c_str(), connect ? "OK" : "FAIL");

		// try get service
		BLERemoteService *pRemoteService = clientInfo.client->getService(serviceUUID);
		if (pRemoteService == nullptr)
		{
			Serial.printf("client FAIL to get remote service.. \n");
			clientInfo.client->disconnect();
			return false;
		}

		pRemoteCharacteristic = pRemoteService->getCharacteristic(characteristicUUID);
		if (pRemoteCharacteristic == nullptr)
		{
			Serial.printf("client FAIL to get remote characteristic.. \n");
			clientInfo.client->disconnect();
			return false;
		}

		if (pRemoteCharacteristic->canNotify())
		{
			pRemoteCharacteristic->registerForNotify(notifyCallback);
		}
		else
		{
			Serial.printf("Remote characteristic can't notify! Client disconnecting \n");
			clientInfo.client->disconnect();
			return false;
		}

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
		if (heart.client->isConnected() == false && heart.doConnect)
		{
			bool connected = connectToServer(heart, hrmDevice, hrServiceUUID, hrMeasureCharacteristicUUID);
			heart.doConnect = false;
		}

		if (ctrlr.client->isConnected() == false && ctrlr.doConnect)
		{
			bool connected = connectToServer(ctrlr, controllerDevice, genericMediaControlServiceUUID, controllerCharacteristicUUID);
			ctrlr.doConnect = false;
		}
	}

	void startScanning()
	{
		if (ctrlr.doScan() || heart.doScan())
		{
			// sendStatus(DISCONNECTED);

			Serial.printf("controllerClient->isConnected(): %s doScan: %d \n",
						  ctrlr.client->isConnected() ? "YES" : "NO", ctrlr.doScan());
			Serial.printf("hrmClient->isConnected(): %s doScan: %d\n",
						  heart.client->isConnected() ? "YES" : "NO", heart.doScan());

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
	}
}