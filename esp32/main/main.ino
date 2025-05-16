#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_AS726x.h>

#define MIC_PIN 34
#define LED_PIN 2

Adafruit_AS726x sensor;

unsigned long startMillis;
float micBuffer[1000];
unsigned long timestampBuffer[1000];
int bufindex = 0;
unsigned int samplingRateHz = 1000;

HardwareSerial UART(1);  // UART1: RX = GPIO16, TX = GPIO17

void setup() {
  Serial.begin(115200);
  Wire.begin();
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  if (!sensor.begin()) {
    Serial.println("AS726x not found!");
    while (true);
  }

  UART.begin(9600, SERIAL_8N1, 16, 17);
  Serial.println("ESP32 UART ready");
}

void loop() {
  static unsigned long lastSampleTime = 0;
  unsigned long now = millis();
  unsigned long interval = 1000 / samplingRateHz;

  if (now - lastSampleTime >= interval && bufindex < 1000) {
    lastSampleTime = now;
    micBuffer[bufindex] = analogRead(MIC_PIN);
    timestampBuffer[bufindex] = now;
    bufindex++;
  }

  if (UART.available()) {
    String cmd = UART.readStringUntil('\n');

    if (cmd.startsWith("SYNC")) {
      double nowSec = millis() / 1000.0;
      UART.write((uint8_t*)&nowSec, sizeof(nowSec));

    } else if (cmd.startsWith("SET")) {
      while (UART.available() < sizeof(double)) delay(1);
      double new_time;
      UART.readBytes((char*)&new_time, sizeof(new_time));
      startMillis = new_time * 1000 - millis();

    } else if (cmd.startsWith("SET_RATE")) {
      String rateStr = cmd.substring(9);
      samplingRateHz = rateStr.toInt();
      Serial.printf("Sampling rate set to %u Hz\n", samplingRateHz);

    } else if (cmd.startsWith("READ")) {
      unsigned long t0 = timestampBuffer[0];
      UART.write((uint8_t*)&t0, sizeof(unsigned long));

      const int chunkSize = 20;
      for (int i = 0; i < 1000; i += chunkSize) {
        int count = min(chunkSize, 1000 - i);
        UART.write((uint8_t*)(micBuffer + i), count * sizeof(float));
        delay(10);
      }
      bufindex = 0;
      Serial.println("Sent timestamp + 1000 samples");
    }
  }
}