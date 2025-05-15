from ble_device import BLEDevice
from sync_protocol import *
import time

ESP32_ADDRESSES = ["38:18:2B:EA:7F:4E", "XX:XX:XX:XX:XX:XX"]
SYNC_CHAR_HANDLE = 0x0010
DATA_CHAR_HANDLE = 0x0012

def main():
    for addr in ESP32_ADDRESSES:
        dev = BLEDevice(addr)
        dev.connect()

        send_time = time.time()
        dev.write(SYNC_CHAR_HANDLE, build_sync_request())
        data = dev.read_notification()
        recv_time = time.time()

        if data:
            esp_time = parse_sync_response(data)
            rtt, offset = estimate_rtt_offset(send_time, recv_time, esp_time)
            print(f"[{addr}] RTT: {rtt:.6f}s, Offset: {offset:.6f}s")

            corrected_time = time.time() + offset
            dev.write(SYNC_CHAR_HANDLE, build_set_time_command(corrected_time))

        dev.write(DATA_CHAR_HANDLE, "READ")
        sensor_data = dev.read_notification()
        if sensor_data:
            print(f"[{addr}] Sensor Data: {sensor_data[:64]}...")

        dev.disconnect()

if __name__ == '__main__':
    main()