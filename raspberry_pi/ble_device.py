from bluepy.btle import Peripheral, DefaultDelegate

class NotificationDelegate(DefaultDelegate):
    def __init__(self):
        super().__init__()
        self.last_data = None

    def handleNotification(self, cHandle, data):
        self.last_data = data

class BLEDevice:
    def __init__(self, address):
        self.address = address
        self.peripheral = None
        self.delegate = NotificationDelegate()

    def connect(self):
        self.peripheral = Peripheral(self.address)
        self.peripheral.setDelegate(self.delegate)

    def write(self, handle, data):
        self.peripheral.writeCharacteristic(handle, data.encode(), withResponse=True)

    def read_notification(self, timeout=2.0):
        if self.peripheral.waitForNotifications(timeout):
            return self.delegate.last_data
        return None

    def disconnect(self):
        if self.peripheral:
            self.peripheral.disconnect()