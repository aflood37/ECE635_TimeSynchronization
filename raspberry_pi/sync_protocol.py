import struct

def estimate_rtt_offset(send_time, recv_time, esp_time):
    rtt = recv_time - send_time
    offset = esp_time - (send_time + rtt / 2)
    return rtt, offset

def build_sync_request():
    return "SYNC"

def parse_sync_response(data):
    return struct.unpack("d", data)[0]

def build_set_time_command(new_time):
    return "SET" + struct.pack("d", new_time)