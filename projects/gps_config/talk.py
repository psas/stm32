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
    f = open('gpslog', 'wb')
    with udp() as sock:
        sock.bind((IP_SELF, PORT_SELF_RX))
        sock.settimeout(TIMEOUT)
        counter = 0
        while True:
            try:
                data, remote_addr = sock.recvfrom(1024)
                if(remote_addr[0] == '10.10.10.40'):
                    counter += 1 % 16
                    f.write(data)
                    print("Received", len(data), ":", "".join("{:02x}".format(ord(c)) for c in data), "from", remote_addr)
            except socket.timeout:
                print ("No response")


if __name__ == "__main__":
    receive()
