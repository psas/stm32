#!/usr/bin/python

from __future__ import print_function
import argparse
from contextlib import contextmanager
import socket
from threading import Thread
import time

IP_DEST = b'192.168.0.192'
PORT_DEST_RX = 35001
IP_SELF = b'192.168.0.191'
PORT_SELF_RX = 35001
TIMEOUT = 1

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
                print("Received", repr(data), "from", remote_addr)
            except socket.timeout:
                print ("No responce")

def send():
    count = 0
    with udp() as sock:
        while True:
            msg = 'Shinybit' + str(count) + '/n'
            sock.sendto(msg.encode('ascii', 'ignore'), (IP_DEST, PORT_DEST_RX))
            print("Sent", repr(msg), "To destination")
            count += 1
            time.sleep(TIMEOUT)


if __name__ == "__main__":
    rx = Thread(target=receive)
    tx = Thread(target=send)
    rx.daemon = True
    tx.daemon = True
    rx.start()
    tx.start()

    while True:
        pass
