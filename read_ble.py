from bleak import BleakClient
import asyncio

ESP32_ADDRESS = "38:18:2B:EA:7F:4E"
CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"

def handle_notify(_, data):
	print("Received:", data.decode())

async def main():
	async with BleakClient(ESP32_ADDRESS) as client:
		connected = await client.is_connected()
		if not connected:
			print("Connection Failed")
			return
		else:
			print("Connected")

		await client.start_notify(CHARACTERISTIC_UUID, handle_notify)
		await asyncio.sleep(15)
		await client.stop_notify(CHARACTERISTIC_UUID)

asyncio.run(main())
