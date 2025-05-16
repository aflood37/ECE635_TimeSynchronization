# ECE635 Time Synchronization via UART (Dual ESP32)

Synchronize and stream timestamped sensor data from two ESP32s to a Raspberry Pi over GPIO UART (no USB).

## UART Wiring

| ESP32 | ESP GPIO | Pi GPIO | Device        |
|--------|-----------|----------|---------------|
| ESP1   | TX=17     | RX=15   | `/dev/serial0` |
| ESP1   | RX=16     | TX=14   |               |
| ESP2   | TX=17     | RX=5    | `/dev/ttyAMA1` |
| ESP2   | RX=16     | TX=4    |               |

Connect GND on both ESP32s to Pi GND.

## Raspberry Pi Setup

Edit `/boot/config.txt`:

```ini
enable_uart=1
dtoverlay=uart1,txd=4,rxd=5
```

Disable serial login:
```bash
sudo raspi-config → Interface Options → Serial → "No"
```

Reboot:
```bash
sudo reboot
```

## Running

```bash
cd raspberry_pi
python3 run_all.py
```
