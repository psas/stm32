#!/usr/bin/env python
from contextlib import contextmanager
import socket

@contextmanager
def udp(bind_ip, bind_port):
    """UDP Socket creator as a context."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((bind_ip, bind_port))
    sock.settimeout(0.25)
    yield sock
    sock.close()


with udp(b'10.0.0.27', 36101) as sock:
    sock.sendto('#LEDS'.encode('ascii', 'ignore'), (b'10.0.0.5', 36100))

