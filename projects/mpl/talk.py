#!/usr/bin/python

from __future__ import print_function
import argparse
from contextlib import contextmanager
import socket
from threading import Thread
import time

IP_SELF = b'10.10.10.10'
PORT_SELF_RX = 36000
TIMEOUT = 1000

@contextmanager
def udp():
    """UDP Socket creator as a context."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    yield sock
    sock.close()

def receive():
    with udp() as sock:
        sock.bind((IP_SELF, PORT_SELF_RX))
        sock.settimeout(TIMEOUT)
        while True:
            try:
                data, remote_addr = sock.recvfrom(50)
                if(remote_addr[0] == '10.10.10.20'):
                    print("Received", repr(data), "from", remote_addr)
            except socket.timeout:
                print ("No responce")


if __name__ == "__main__":
    rx = Thread(target=receive)
    rx.daemon = True
    rx.start()

    while True:
        pass
