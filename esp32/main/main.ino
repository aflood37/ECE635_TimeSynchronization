//#include <Arduino.h>
//#include <BLEDevice.h>
//#include <BLEServer.h>
//#include <BLEUtils.h>
//#include <BLE2902.h>
//#include <Wire.h>
//#include <Adafruit_AS726x.h>
//
//#define MIC_PIN 34
//#define LED_PIN 2
//
//Adafruit_AS726x sensor;
//
//BLECharacteristic *syncChar;
//BLECharacteristic *dataChar;
//BLEServer *server;
//
//unsigned long startMillis;
//float micBuffer[1000];
//float lightBuffer[1000];
//int bufindex = 0;
//
//bool deviceConnected = false;
//
//// === BLE Server Callbacks ===
//class MyServerCallbacks : public BLEServerCallbacks {
//  void onConnect(BLEServer *pServer) override {
//    Serial.println("Client connected");
//    digitalWrite(LED_PIN, HIGH);
//    deviceConnected = true;
//  }
//
//  void onDisconnect(BLEServer *pServer) override {
//    Serial.println("Client disconnected, restarting advertising");
//    digitalWrite(LED_PIN, LOW);
//    deviceConnected = false;
//    pServer->getAdvertising()->start();
//  }
//};
//
//// === BLE Characteristic Callback for SYNC messages ===
//class SyncCallbacks : public BLECharacteristicCallbacks {
//  void onWrite(BLECharacteristic *pChar) override {
//    String value = pChar->getValue();  // Arduino String
//
//    if (value.startsWith("SYNC")) {
//      double now = millis() / 1000.0;
//      pChar->setValue((uint8_t *)&now, sizeof(now));
//      pChar->notify();
//    } else if (value.startsWith("SET")) {
//      double new_time;
//      value = value.substring(3);  // Extract bytes after "SET"
//      memcpy(&new_time, value.c_str(), sizeof(new_time));
//      startMillis = new_time * 1000 - millis();
//    }
//  }
//};
//
//// === Read microphone + light sensor into buffers ===
//void sampleSensors() {
//  if (bufindex < 1000) {
//    micBuffer[bufindex] = analogRead(MIC_PIN);
//    lightBuffer[bufindex] = sensor.dataReady() ? sensor.readTemperature() : -1.0;
//    bufindex++;
//  }
//}
//
//void setup() {
//  Serial.begin(115200);
//  Wire.begin();
//  pinMode(LED_PIN, OUTPUT);
//  digitalWrite(LED_PIN, LOW);
//
//  if (!sensor.begin()) {
//    Serial.println("AS726x not found!");
//    while (true);  // halt
//  }
//
//  BLEDevice::init("ESP32_TimeNode");
//  Serial.print("Device address: ");
//  Serial.println(BLEDevice::getAddress().toString().c_str());
//
//  server = BLEDevice::createServer();
//  server->setCallbacks(new MyServerCallbacks());
//
//  BLEService *service = server->createService("12345678-1234-1234-1234-1234567890ab");
//
//  syncChar = service->createCharacteristic(
//    "abcdefab-1234-1234-1234-abcdefabcdef",
//    BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY
//  );
//  syncChar->setCallbacks(new SyncCallbacks());
//  syncChar->addDescriptor(new BLE2902());
//
//  dataChar = service->createCharacteristic(
//    "fedcba98-7654-3210-fedc-ba9876543210",
//    BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY
//  );
//  dataChar->addDescriptor(new BLE2902());
//
//  service->start();
//  server->getAdvertising()->start();
//  Serial.println("ESP advertising");
//}
//
//void loop() {
//  sampleSensors();
//  delay(1);
//
//  // Failsafe: Restart advertising if somehow stopped
//  static bool wasAdvertising = false;
//  if (!deviceConnected && !wasAdvertising) {
//    Serial.println("Restarting advertising from loop");
//    server->getAdvertising()->start();
//    wasAdvertising = true;
//  } else if (deviceConnected) {
//    wasAdvertising = false;
//  }
//
//  // Send buffer in chunks
//  if (bufindex >= 1000) {
//    const int chunkSize = 20;
//    for (int i = 0; i < 1000; i += chunkSize) {
//      int thisChunk = min(chunkSize, 1000 - i);
//      dataChar->setValue((uint8_t *)(micBuffer + i), thisChunk * sizeof(float));
//      dataChar->notify();
//      delay(10);
//    }
//
//    bufindex = 0;
//    Serial.println("Buffer sent");
//  }
//}

//#include <BLEDevice.h>
//#include <BLEServer.h>
//#include <BLEUtils.h>
//#include <BLE2902.h>
//#include <BLE2901.h>
//
//BLEServer *pServer = NULL;
//BLECharacteristic *pCharacteristic = NULL;
//BLE2901 *descriptor_2901 = NULL;
//
//bool deviceConnected = false;
//bool oldDeviceConnected = false;
//uint32_t value = 0;
//
//// See the following for generating UUIDs:
//// https://www.uuidgenerator.net/
//
//#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
//#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
//
//class MyServerCallbacks : public BLEServerCallbacks {
//  void onConnect(BLEServer *pServer) {
//    deviceConnected = true;
//  };
//
//  void onDisconnect(BLEServer *pServer) {
//    deviceConnected = false;
//  }
//};
//
//void setup() {
//  Serial.begin(115200);
//
//  // Create the BLE Device
//  BLEDevice::init("ESP32");
//
//  // Create the BLE Server
//  pServer = BLEDevice::createServer();
//  pServer->setCallbacks(new MyServerCallbacks());
//
//  // Create the BLE Service
//  BLEService *pService = pServer->createService(SERVICE_UUID);
//
//  // Create a BLE Characteristic
//  pCharacteristic = pService->createCharacteristic(
//    CHARACTERISTIC_UUID,
//    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE
//  );
//
//  // Creates BLE Descriptor 0x2902: Client Characteristic Configuration Descriptor (CCCD)
//  pCharacteristic->addDescriptor(new BLE2902());
//  // Adds also the Characteristic User Description - 0x2901 descriptor
//  descriptor_2901 = new BLE2901();
//  descriptor_2901->setDescription("My own description for this characteristic.");
//  descriptor_2901->setAccessPermissions(ESP_GATT_PERM_READ);  // enforce read only - default is Read|Write
//  pCharacteristic->addDescriptor(descriptor_2901);
//
//  // Start the service
//  pService->start();
//
//  // Start advertising
//  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
//  pAdvertising->addServiceUUID(SERVICE_UUID);
//  pAdvertising->setScanResponse(false);
//  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
//  BLEDevice::startAdvertising();
//  Serial.println("Waiting a client connection to notify...");
//}
//
//void loop() {
//  // notify changed value
//  if (deviceConnected) {
//    pCharacteristic->setValue((uint8_t *)&value, 4);
//    pCharacteristic->notify();
//    value++;
//    delay(500);
//  }
//  // disconnecting
//  if (!deviceConnected && oldDeviceConnected) {
//    delay(500);                   // give the bluetooth stack the chance to get things ready
//    pServer->startAdvertising();  // restart advertising
//    Serial.println("start advertising");
//    oldDeviceConnected = deviceConnected;
//  }
//  // connecting
//  if (deviceConnected && !oldDeviceConnected) {
//    // do stuff here on connecting
//    oldDeviceConnected = deviceConnected;
//  }
//}

#include <Arduino.h>
#include "BluetoothSerial.h"

#define LED_PIN 2  // Onboard LED

BluetoothSerial SerialBT;
bool isConnected = false;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  // Start Bluetooth with a custom device name
  SerialBT.begin("ESP32_BT"); 
  Serial.println("Bluetooth started. Waiting for connection...");
}

void loop() {
  // Check connection status
  if (SerialBT.hasClient()) {
    if (!isConnected) {
      isConnected = true;
      Serial.println("Client connected!");
      digitalWrite(LED_PIN, HIGH);  // Turn on LED to indicate connection
    }

    // Echo any received data back
    while (SerialBT.available()) {
      char c = SerialBT.read();
      SerialBT.write(c);  // Echo back
      Serial.write(c);    // Print to serial monitor
    }

  } else {
    if (isConnected) {
      isConnected = false;
      Serial.println("Client disconnected.");
      digitalWrite(LED_PIN, LOW);  // Turn off LED
    }
  }

  delay(20);  // Short delay to avoid spamming CPU
}
