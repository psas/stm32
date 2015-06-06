#!/usr/bin/python

from __future__ import print_function
from collections import OrderedDict
from contextlib import contextmanager
import argparse
import json
import socket
from registers import *
import struct

IP_SELF = b'10.10.10.10'
PORT_SELF_RX = 36000
TIMEOUT = 1000

@contextmanager
def tcp(ip, port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind(('', 0))
    sock.connect((ip, port))
    yield sock
    sock.close()

@contextmanager
def udp():
    """UDP Socket creator as a context."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    yield sock
    sock.close()

def receive():
    with udp() as sock:
        sock.bind((IP_SELF, PORT_SELF_RX))
        sock.connect((b'10.10.10.40', 35050))
        sock.settimeout(TIMEOUT)

        counter = 0
        for i in xrange(1998*2):
            try:
                data = sock.recv(1028)
                expected_counter = counter + 1
                counter = struct.unpack("I", data[0:4])[0]
                if counter != expected_counter:
                    print("Smooth stream starting at: %i (expected %i)" % (counter, expected_counter))
            except socket.timeout:
                print ("No response")
                break
            yield data[4:]


def cmd(string):
    with tcp(b'10.10.10.40', 23) as sock:
        data = '{}\r\n'.format(string).encode('ascii', 'ignore')
        sock.send(data)

initial = join_registers((
    CONF1(
        CHIPEN=0b1,
        IDLE=0b0,
        ILNA1=0b1111,
        ILNA2=0b00,
        ILO=0b11,
        IMIX=0b11,
        MIXPOLE=0b0,
        LNAMODE=0b10,
        MIXEN=0b1,
        ANTEN=0b0,
        FCEN=0b000000,
        FBW=0b00,
        F3OR5=0b0,
        FCENX=0b0,
        FGAIN=0b1,
    ),
    CONF2(
        IQEN=0b1,
        GAINREF=0b10101010,
        AGCMODE=0b01,
        FORMAT=0b01,
        BITS=0b010,
        DRVCFG=0b00,
        LOEN=0b1,
        DIEID=0b00,
    ),
    CONF3(
        GAININ=0b111010,
        FSLOWEN=0b1,
        HILOADEN=0b0,
        ADCEN=0b1,
        DRVEN=0b1,
        FOFSTEN=0b1,
        FILTEN=0b1,
        FHIPEN=0b0,
        PGAIEN=0b1,
        PGAQEN=0b1,
        STRMEN=0b0,
        STRMSTART=0b0,
        STRMSTOP=0b0,
        STRMCOUNT=0b111,
        STRMBITS=0b11,
        STAMPEN=0b0,
        TIMESYNCEN=0b0,
        DATSYNCEN=0b0,
        STRMRST=0b0,
    ),
    PLLCONF(
        VCOEN=0b1,
        IVCO=0b0,
        REFOUTEN=0b1,
        REFDIV=0b01,
        IXTAL=0b01,
        XTALCAP=0b10000,
        LDMUX=0b0000,
        ICP=0b1,
        PFDEN=0b0,
        CPTEST=0b000,
        INT_PLL=0b1,
        PWRSAV=0b0,
    ),
    DIV(
        NDIV=1540,
        RDIV=16,
    ),
    CLK(
        L_CNT=1024,
        M_CNT=1024,
        FCLKIN=0,
        ADCCLK=0,
        SERCLK=1,
        MODE=0,
    ),
))
'''
    CONF1(
        FCEN=0b111111,
        FCENX=0b1,
    ),
    CONF2(
        AGCMODE=0b01,
    ),
    CONF3(
        FSLOWEN=0b1,
    ),
    '''

mask = join_registers((
))

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='GPSv3 talker')
    parser.add_argument('-v', action='store_true', help='Print each packet as it comes in')
    parser.add_argument('-d', action='store_true', help='Assume the GPS CPLD is in debug mode and check the stream')
    args = parser.parse_args()


    for i, merged_regs in enumerate(fuzz_registers(initial, mask)):
        regs = split_registers(merged_regs)

        with open('data/gpslog{:04d}.json'.format(i), 'wb') as jsonfile:
            json.dump(OrderedDict((idx, OrderedDict(reversed(list(reg.items())))) for idx, reg in regs), jsonfile, indent=2)

        with open('data/gpslog{:04d}'.format(i), 'wb') as f:
            end = 255
            packetcounter = 0

            for addr, reg in regs:
                cmd('#CONF%x%07x\r\n' % (addr, reg.base))

            for data in receive():
                f.write(data)

                if args.v or args.d:
                    binary = [ord(c) for c in data]

                if args.v:
                    print("Received", len(data), ":", "".join("{:02x}".format(c) for c in binary), "from", remote_addr)

                if args.d:
                    packetcounter += 1
                    if (end+1) % 256 != binary[0]:
                        print("Packet {}(l{}): boundary error: {},{}"
                                .format(packetcounter, len(binary), end, binary[0]))
                    for index, num in enumerate(binary[:-1]):
                        if binary[index + 1] != (num + 1) % 256:
                            print("Packet {}(l{}): out of order sequence at index {}: {},{}"
                                    .format(packetcounter, len(binary), index, binary[index], binary[index+1]))
                    end = binary[-1]


