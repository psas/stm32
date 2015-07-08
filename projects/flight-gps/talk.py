#!/usr/bin/python

from __future__ import print_function
import argparse
from contextlib import contextmanager
import socket
import time
import sys


IP_SELF = b'10.10.10.10'
PORT_SELF_RX = 36000
TIMEOUT = 10

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
            data = []
            try:
                data, remote_addr = sock.recvfrom(50)
                if(remote_addr[0] == '10.10.10.40'):
                    #writeout = data[4:]
                    #writeout.replace('\0', '')
                    sys.stdout.write(repr(data))
                else:
                    print(remote_addr[0])
            except socket.timeout:
                print ("No response")


if __name__ == "__main__":
    receive()
