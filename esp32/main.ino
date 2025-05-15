#inlcude <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Wire.h>
#include "AS726X.h"

AS726X sensor;
#define MIC_PIN 34

BLECharacteristic *syncChar;
BLECharacteristic *dataChar;

unsigned long startMillis;
float micBuffer[1000];
float lightBuffer[1000];
int index = 0;

void sampleSensors() {
  if (index < 1000) {
    micBuffer[index] = analogRead(MIC_PIN);
    lightBuffer[index] = sensor.readTemperature(); // placeholder
    index++;
  }
}

class SyncCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pChar) {
    std::string value = pChar->getValue();
    if (value.substr(0, 4) == "SYNC") {
      double now = millis() / 1000.0;
      pChar->setValue((uint8_t*)&now, sizeof(now));
      pChar->notify();
    } else if (value.substr(0, 3) == "SET") {
      double new_time;
      memcpy(&new_time, value.data() + 3, sizeof(new_time));
      startMillis = new_time * 1000 - millis();
    }
  }
};

void setup() {
  Serial.begin(115200);
  Wire.begin();
  sensor.begin();
  BLEDevice::init("ESP32_TimeNode");
  BLEServer *server = BLEDevice::createServer();

  syncChar = server->createCharacteristic("0001", BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
  syncChar->setCallbacks(new SyncCallbacks());
  syncChar->addDescriptor(new BLE2902());

  dataChar = server->createCharacteristic("0002", BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
  dataChar->addDescriptor(new BLE2902());

  server->getAdvertising()->start();
}

void loop() {
  sampleSensors();
  delay(1);
  if (index >= 1000) {
    dataChar->setValue((uint8_t*)micBuffer, sizeof(micBuffer));
    dataChar->notify();
    index = 0;
  }
