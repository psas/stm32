#!/usr/bin/python

from __future__ import print_function
from contextlib import contextmanager
import argparse
import socket

IP_SELF = b'10.10.10.10'
PORT_SELF_RX = 36000
TIMEOUT = 1000

@contextmanager
def udp():
    """UDP Socket creator as a context."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    yield sock
    sock.close()

def receive(verbose, debug):
    with udp() as sock, open('gpslog', 'wb') as f:
        sock.bind((IP_SELF, PORT_SELF_RX))
        sock.settimeout(TIMEOUT)

        end = 255
        packetcounter = 0

        while True:
            try:
                data, remote_addr = sock.recvfrom(1024)
            except socket.timeout:
                print ("No response")
                continue

            if(remote_addr[0] != '10.10.10.40'):
                continue
            if verbose or debug:
                binary = [ord(c) for c in data]

            if verbose:
                print("Received", len(data), ":", "".join("{:02x}".format(c) for c in binary), "from", remote_addr)

            if debug:
                packetcounter += 1
                if (end+1) % 256 != binary[0]:
                    print("Packet {}(l{}): boundary error: {},{}"
                            .format(packetcounter, len(binary), end, binary[0]))
                for index, num in enumerate(binary[:-1]):
                    if binary[index + 1] != (num + 1) % 256:
                        print("Packet {}(l{}): out of order sequence at index {}: {},{}"
                                .format(packetcounter, len(binary), index, binary[index], binary[index+1]))
                end = binary[-1]

            f.write(data)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='GPSv3 talker')
    parser.add_argument('-v', action='store_true', help='Print each packet as it comes in')
    parser.add_argument('-d', action='store_true', help='Assume the GPS CPLD is in debug mode and check the stream')
    args = parser.parse_args()
    receive(args.v, args.d)
