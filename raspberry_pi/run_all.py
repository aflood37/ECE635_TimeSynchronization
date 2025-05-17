from esp_device import ESPDevice
import time

esp_ports = {
    "esp1": "/dev/serial0",     # UART0 (GPIO14/15)
    "esp2": "/dev/ttyAMA2"      # UART1 (GPIO4/5)
}

def safe_read_samples(dev, retries=3):
    for i in range(retries):
        try:
            print(f"[{dev.name}] Attempt {i+1} to read samples...")
            return dev.read_samples(sampling_interval_ms=1.0)
        except RuntimeError as e:
            print(f"[{dev.name}] Warning: {e}")
            time.sleep(1)
    raise RuntimeError(f"[{dev.name}] Failed to read samples after {retries} attempts")

devices = {name: ESPDevice(port, name=name) for name, port in esp_ports.items()}

try:
    for name, dev in devices.items():
        print(f"[{name}] SYNC...")
        info = dev.sync_time()
        dev.set_time(time.time() + info["offset"])
        dev.set_sampling_rate(1000)
        print(f"[{name}] RTT={info['rtt']:.4f}, Offset={info['offset']:.4f}")


    for name, dev in devices.items():
        print(f"[{name}] Reading...")
        data = safe_read_samples(dev)
        print(f"[{name}] First 3 samples:", data[:3])

finally:
    for dev in devices.values():
        dev.close()
