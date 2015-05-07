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
        end = 0
        packetcounter = 0
        while True:
            try:
                data, remote_addr = sock.recvfrom(1024)
                if(remote_addr[0] == '10.10.10.40'):
                    packetcounter += 1
                    binary = [ord(c) for c in data]

                    #if (end+1) % 256 != binary[0]:
                    #    print("packet {} boudary error: {}, {},{}".format(packetcounter, end, (end+1) % 256, binary[0]))
                    #for index, num in enumerate(binary[:-1]):
                    #    if binary[index + 1] != (num + 1) % 256:
                    #        print("out of order sequence found at packet {},{} index {}: {},{}".format(packetcounter, len(binary), index, binary[index], binary[index+1]))
                    #end = binary[-1]
                    print("Received", len(data), ":", "".join("{:02x}".format(c) for c in binary), "from", remote_addr)
                    f.write(data)
            except socket.timeout:
                print ("No response")


if __name__ == "__main__":
    receive()
