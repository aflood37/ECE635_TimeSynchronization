#include <Arduino.h>
#include <Wire.h>

#define MIC_PIN 34
#define LED_PIN 2
#define BUFFER_SIZE 100

unsigned long startMillis;
float micBuffer[BUFFER_SIZE];
unsigned long timestampBuffer[BUFFER_SIZE];
int bufindex = 0;
unsigned int samplingRateHz = 1000;

HardwareSerial UART(1);  // UART1: RX = GPIO16, TX = GPIO17

void sampleSensors() {
  if (bufindex < BUFFER_SIZE) {
    micBuffer[bufindex] = analogRead(MIC_PIN);
    timestampBuffer[bufindex] = millis();
    bufindex++;
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  UART.begin(115200, SERIAL_8N1, 16, 17);
  Serial.println("ESP32 UART ready");
}

void handleCommand(const String& raw_cmd) {
  String cmd = raw_cmd;
  cmd.trim();

  if (!cmd.startsWith("CMD:")) {
    Serial.printf("Invalid command (missing CMD:): '%s'\n", cmd.c_str());
    while (UART.available()) UART.read();  // flush junk
    return;
  }

  cmd.remove(0, 4);  // Strip "CMD:"

  if (cmd == "SYNC") {
    double nowSec = millis() / 1000.0;
    UART.write((uint8_t*)&nowSec, sizeof(nowSec));
    Serial.println("SYNC sent");

  } else if (cmd == "SET") {
    while (UART.available() < sizeof(double)) delay(1);
    double new_time;
    UART.readBytes((char*)&new_time, sizeof(new_time));
    startMillis = new_time * 1000 - millis();
    Serial.printf("SET received: new_time = %.3f\n", new_time);
    while (UART.available()) UART.read();  // Flush trailing junk

  } else if (cmd.startsWith("SET_RATE:")) {
    String rateStr = cmd.substring(9);
    samplingRateHz = rateStr.toInt();
    Serial.printf("Sampling rate set to %u Hz\n", samplingRateHz);

  } else if (cmd == "READ") {
    if (bufindex < BUFFER_SIZE) {
      UART.println("NOT_READY\n");
    } else {
      unsigned long t0 = timestampBuffer[0];
      UART.write((uint8_t*)&t0, sizeof(t0));

      const int chunkSize = 10;
      for (int i = 0; i < BUFFER_SIZE; i += chunkSize) {
        int count = min(chunkSize, BUFFER_SIZE - i);
        UART.write((uint8_t*)(micBuffer + i), count * sizeof(float));
        delay(5);
      }

      bufindex = 0;
      Serial.println("Buffer sent");
    }

  } else if (cmd == "PING") {
    UART.println("PONG\n");

  } else {
    Serial.printf("Unknown CMD: '%s'\n", cmd.c_str());
    while (UART.available()) UART.read();  // flush junk
  }
}

void loop() {
  sampleSensors();
  delay(1000 / samplingRateHz);

  static String cmd = "";

  while (UART.available()) {
    char c = UART.read();
    if (c == '\n') {
      if (cmd.length() > 0) {
        handleCommand(cmd);
      }
      cmd = "";
    } else {
      cmd += c;
    }
  }
}
