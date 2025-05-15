from bleak import BleakClient
import asyncio

ADDRESS = "38:18:2B:EA:7F:4E"  # your ESP32 address

async def main():
    client = BleakClient(ADDRESS)
    try:
        await client.connect()
        print("Connected?", await client.is_connected())
        services = await client.get_services()
        for s in services:
            print(s)
    except Exception as e:
        print("Connection error:", e)
    finally:
        await client.disconnect()

asyncio.run(main())
