from bleak import BleakScanner
import asyncio

async def main():
	devices = await BleakScanner.discover()
	for d in devices:
		print(d)

asyncio.run(main())
