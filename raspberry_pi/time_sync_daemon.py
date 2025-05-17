import threading
import queue
import time
import logging
import matplotlib.pyplot as plt
import numpy as np
from esp_device import ESPDevice
from config import *
from pathlib import Path
import random
import csv
from matplotlib.animation import FuncAnimation

Path(PLOT_DIR).mkdir(exist_ok=True)

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
    handlers=[
        logging.FileHandler("time_sync.log"),
        logging.StreamHandler()
    ]
)

offset_history = []
delay_diff_history = []
timestamps = []

class SerialReaderThread(threading.Thread):
    def __init__(self, port, name, queue):
        super().__init__(daemon=True)
        self.device = ESPDevice(port=port, baudrate=BAUD_RATE)
        self.name = name
        self.queue = queue

    def run(self):
        while True:
            try:
                time.sleep(random.uniform(0.005, 0.015))
                recv_time = time.time()
                base_ts, samples = self.device.read_samples()
                self.queue.put((recv_time, base_ts, samples))
                logging.info(f"{self.name}: Received {len(samples)} samples at {recv_time}")
            except Exception as e:
                logging.warning(f"{self.name} read failed: {e}")
                time.sleep(0.1)

class ClockOffsetEstimator:
    def estimate_offset(self, esp1_data, esp2_data):
        (recv1, ts1, s1) = esp1_data
        (recv2, ts2, s2) = esp2_data

        if len(s1) != len(s2):
            min_len = min(len(s1), len(s2))
            s1, s2 = s1[:min_len], s2[:min_len]

        corr = np.correlate(s1 - np.mean(s1), s2 - np.mean(s2), mode="full")
        offset_samples = np.argmax(corr) - (len(s1) - 1)
        sample_period = 1 / 1000
        offset_seconds = offset_samples * sample_period

        uart_delay1 = recv1 - (ts1 / 1000.0)
        uart_delay2 = recv2 - (ts2 / 1000.0)

        corrected_offset = offset_seconds + (uart_delay2 - uart_delay1)

        logging.info(f"Raw offset: {offset_seconds:.6f}s, UART delay diff: {(uart_delay2 - uart_delay1):.6f}s, Corrected: {corrected_offset:.6f}s")

        now = time.time()
        offset_history.append(corrected_offset)
        delay_diff_history.append(uart_delay2 - uart_delay1)
        timestamps.append(now)
        if len(offset_history) > 100:
            offset_history.pop(0)
            delay_diff_history.pop(0)
            timestamps.pop(0)

        with open(CSV_LOG, "a", newline="") as f:
            writer = csv.writer(f)
            writer.writerow([now, offset_seconds, uart_delay2 - uart_delay1, corrected_offset])

        if ENABLE_PLOTS:
            self.plot_alignment(s1, s2, offset_samples)

        return corrected_offset

    def plot_alignment(self, s1, s2, offset):
        plt.figure(figsize=(10, 4))
        plt.plot(s1, label="ESP1")
        plt.plot(np.roll(s2, offset), label=f"ESP2 (shifted {offset})")
        plt.title("Signal Alignment")
        plt.legend()
        filename = f"{PLOT_DIR}/alignment_{int(time.time())}.png"
        plt.savefig(filename)
        plt.close()
        logging.debug(f"Saved alignment plot to {filename}")

def live_plot():
    fig, ax = plt.subplots()
    line1, = ax.plot([], [], label="Clock Offset (s)")
    line2, = ax.plot([], [], label="UART Delay Diff (s)")
    ax.legend()
    ax.set_ylim(-0.1, 0.1)
    ax.set_xlabel("Time")
    ax.set_ylabel("Seconds")

    def update(frame):
        if not timestamps:
            return line1, line2
        line1.set_data(timestamps, offset_history)
        line2.set_data(timestamps, delay_diff_history)
        ax.set_xlim(timestamps[0], timestamps[-1])
        return line1, line2

    ani = FuncAnimation(fig, update, interval=1000)
    plt.show()

def main():
    q1 = queue.Queue()
    q2 = queue.Queue()

    reader1 = SerialReaderThread(ESP1_PORT, "ESP1", q1)
    reader2 = SerialReaderThread(ESP2_PORT, "ESP2", q2)
    reader1.start()
    reader2.start()

    estimator = ClockOffsetEstimator()

    if ENABLE_PLOTS:
        plot_thread = threading.Thread(target=live_plot, daemon=True)
        plot_thread.start()

    try:
        with open(CSV_LOG, "x", newline="") as f:
            writer = csv.writer(f)
            writer.writerow(["timestamp", "raw_offset", "uart_delay_diff", "corrected_offset"])
    except FileExistsError:
        pass

    while True:
        try:
            data1 = q1.get()
            data2 = q2.get()
            estimator.estimate_offset(data1, data2)
            time.sleep(2)
        except Exception as e:
            logging.error(f"Error during estimation: {e}")
            time.sleep(2)

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        logging.info("Time sync daemon terminated.")
