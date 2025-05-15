# ECE635 Time Synchronization via Sensing Final Project

This is a final project for a graduate-level course on embedded systems and wireless communication. The goal of this project is to implement a time synchronization protocol over BLE between a Raspberry Pi 5 and two ESP32 devices that collect environmental sensor data.

## Project Overview

- **Devices:** Raspberry Pi 5 + Two ESP32s
- **Sensors on ESP32:**
  - Electret Microphone (ambient sound)
  - AS7262 spectral light sensor (ambient light)
- **BLE Communication:**
  - ESP32s advertise as GATT servers
  - Raspberry Pi acts as central client (using `bluepy`)
- **Purpose:**
  - Collect time-stamped 1-second buffers of sensor data
  - Transmit to Raspberry Pi
  - Estimate clock drift and BLE network latency
  - Synchronize ESP32 clocks to the Pi's time base

## Features

- Continuous sampling and buffering of microphone and light sensor data on ESP32s
- BLE GATT characteristics for:
  - Receiving synchronization commands (`SYNC`, `SET`)
  - Sending buffered sensor data
- Round-trip time (RTT) and offset estimation algorithm on Pi
- Simple protocol to align clocks

## Folder Structure

```
ble_time_sync_project/
├── raspberry_pi/         # Code for Raspberry Pi
│   ├── bluepy_read.py    # Main entry point
│   ├── ble_device.py     # BLE communication handler
│   ├── sync_protocol.py  # Sync protocol (SYNC, SET)
├── esp32/                # Code for ESP32 (Arduino)
│   └── main.ino          # BLE server with sensor readout
```

## How to Run

### Raspberry Pi
1. Install dependencies:
   ```bash
   sudo apt install python3-pip
   pip3 install bluepy
   ```
2. Run the script:
   ```bash
   cd raspberry_pi
   python3 bluepy_read.py
   ```

### ESP32
1. Use Arduino IDE with ESP32 board package installed
2. Connect AS7262 via I2C and microphone to analog pin
3. Upload `main.ino` to both ESP32s

## Notes

- Uses `bluepy` instead of `bleak` for compatibility with Raspberry Pi
- Light sensor integration is pluggable; replace AS7262 with TSL2591 or phototransistors as needed

## References
- Fatima Anwar et al., "Clock Synchronization for BLE-Connected Devices" – UMass Amherst

## License
This project is for academic purposes only.
