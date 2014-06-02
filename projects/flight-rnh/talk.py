#!/usr/bin/python

from __future__ import print_function
import argparse
from contextlib import contextmanager
import socket
from threading import Thread
import time
import sys

IP_DEST = b'10.0.0.5'
PORT_DEST_RX = 36100
IP_SELF = b'10.0.0.146'
PORT_SELF_RX = 35001
TIMEOUT = 5

@contextmanager
def udp():
    """UDP Socket creator as a context."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    yield sock
    sock.close()

if __name__ == "__main__":
    with udp() as sock:
        sock.bind((IP_SELF, PORT_SELF_RX))
        sock.settimeout(TIMEOUT)
        msg = sys.argv[1]
        sock.sendto(msg.encode('ascii', 'ignore'), (IP_DEST, PORT_DEST_RX))
        print("Sent", repr(msg), "To destination")
        try:
            data, remote_addr = sock.recvfrom(50)
            print("Received", repr(data), "from", remote_addr)
        except socket.timeout:
            print ("No response")

