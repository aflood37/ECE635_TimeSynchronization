import struct

def build_sync_request():
    return b"SYNC\n"

def build_set_time_command(epoch_time):
    return b"SET\n" + struct.pack('<d', epoch_time)

def parse_sync_response(resp):
    return struct.unpack('<d', resp)[0]

def estimate_rtt_offset(t_send, t_recv, t_esp):
    rtt = t_recv - t_send
    offset = t_esp - (t_send + rtt / 2)
    return rtt, offset
