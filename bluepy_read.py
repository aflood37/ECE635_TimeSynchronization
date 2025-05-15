from bluepy.btle import Peripheral, UUID

ESP32_ADDRESS = "38:18:2B:EA:7F:4E"
CHAR_UUID = UUID("beb5483e-36e1-4688-b7f5-ea07361b26a8")


# Connect to ESP32
p = Peripheral(ESP32_ADDRESS)
try:
    # Optional: list services and characteristics
    for svc in p.services:
        print(f"Service {svc.uuid}")
        for ch in svc.getCharacteristics():
            print(f"  Characteristic {ch.uuid}")

    # Read a specific characteristic
    ch = p.getCharacteristics(uuid=CHAR_UUID)[0]
    if ch.supportsRead():
        value = ch.read()
        print("Received:", value.decode())
finally:
    p.disconnect()
