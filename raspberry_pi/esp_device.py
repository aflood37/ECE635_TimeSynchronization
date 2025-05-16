import struct
import time
import serial

class ESPDevice:
    def __init__(self, port, baud=9600, name=None):
        self.ser = serial.Serial(port, baudrate=baud, timeout=2)
        self.name = name or port

    def set_sampling_rate(self, rate_hz):
        self.ser.write(f"SET_RATE {rate_hz}\n".encode())
        time.sleep(0.1)

    def sync_time(self):
        t_send = time.time()
        self.ser.write(b"SYNC\n")
        esp_bytes = self.ser.read(8)
        t_recv = time.time()

        if len(esp_bytes) != 8:
            raise RuntimeError("Sync failed")
        esp_time = struct.unpack('<d', esp_bytes)[0]
        rtt = t_recv - t_send
        offset = esp_time - (t_send + rtt / 2)
        return {"offset": offset, "rtt": rtt}

    def set_time(self, epoch):
        self.ser.write(b"SET\n" + struct.pack('<d', epoch))

    def read_samples(self, sampling_interval_ms=1.0):
        self.ser.write(b"READ\n")
        base_ts_raw = self.ser.read(4)
        data_raw = self.ser.read(1000 * 4)

        if len(base_ts_raw) < 4 or len(data_raw) < 1000 * 4:
            raise RuntimeError("Data read incomplete")

        base_ts = int.from_bytes(base_ts_raw, 'little')
        values = list(struct.unpack('<1000f', data_raw))
        timestamps = [base_ts + i * sampling_interval_ms for i in range(1000)]
        return list(zip(timestamps, values))

    def close(self):
        self.ser.close()