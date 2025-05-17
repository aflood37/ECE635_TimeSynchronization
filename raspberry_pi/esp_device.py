import struct
import time
import serial
from config import *

class ESPDevice:
    def __init__(self, port, baudrate=115200, name=None):
        self.ser = serial.Serial(port, baudrate=baudrate, timeout=2)
        self.name = name or port

    def set_sampling_rate(self, rate_hz):
        self.ser.write(f"CMD:SET_RATE {rate_hz}\n".encode())
        time.sleep(0.1)

    def sync_time(self):
        t_send = time.time()
        self.ser.write(b"CMD:SYNC\n")
        time.sleep(0.01)
        esp_bytes = self.ser.read(8)
        t_recv = time.time()

        if len(esp_bytes) != 8:
            raise RuntimeError("Sync failed")
        esp_time = struct.unpack('<d', esp_bytes)[0]
        rtt = t_recv - t_send
        offset = esp_time - (t_send + rtt / 2)
        return {"offset": offset, "rtt": rtt}

    def set_time(self, epoch):
        self.ser.write(b"CMD:SET\n" + struct.pack('<d', epoch))

    def read_samples(self, sampling_interval_ms=1.0):
        self.ser.reset_input_buffer()  # Flushes any residual garbage before reading
        self.ser.write(b"CMD:READ\n")
        print(f"[{self.name}] Waiting for 4-byte base timestamp...")
        time.sleep(0.05)
        base_ts_raw = self.ser.read(4)
        print(f"[{self.name}] Got {len(base_ts_raw)} bytes for base_ts")

        print(f"[{self.name}] Waiting for {4 * BUFFER_SAMPLES} bytes of data...")
        time.sleep(0.05)
        data_raw = self.ser.read(BUFFER_SAMPLES * 4)
        print(f"[{self.name}] Got {len(data_raw)} bytes for data")


        if len(base_ts_raw) < 4 or len(data_raw) < BUFFER_SAMPLES * 4:
            raise RuntimeError("Data read incomplete")

        base_ts = int.from_bytes(base_ts_raw, 'little')
        values = list(struct.unpack(f'<{BUFFER_SAMPLES}f', data_raw))
        timestamps = [base_ts + i * sampling_interval_ms for i in range(BUFFER_SAMPLES)]
        # return list(zip(timestamps, values))
        return base_ts, values

    def close(self):
        self.ser.close()
