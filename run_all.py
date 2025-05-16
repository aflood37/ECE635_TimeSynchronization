from raspberry_pi.esp_device import ESPDevice
import time

esp_ports = {
    "esp1": "/dev/serial0",     # UART0 (GPIO14/15)
    "esp2": "/dev/ttyAMA1"      # UART1 (GPIO4/5)
}

devices = {name: ESPDevice(port, name=name) for name, port in esp_ports.items()}

try:
    for name, dev in devices.items():
        print(f"[{name}] SYNC...")
        info = dev.sync_time()
        dev.set_time(time.time() + info["offset"])
        dev.set_sampling_rate(1000)
        print(f"[{name}] RTT={info['rtt']:.4f}, Offset={info['offset']:.4f}")

    time.sleep(2)

    for name, dev in devices.items():
        print(f"[{name}] Reading...")
        data = dev.read_samples(sampling_interval_ms=1.0)
        print(f"[{name}] First 3 samples:", data[:3])

finally:
    for dev in devices.values():
        dev.close()
