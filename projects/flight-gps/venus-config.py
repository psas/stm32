#!/usr/bin/python
import serial, time
from struct import pack
from contextlib import contextmanager
import socket
"""
Connects to a SkyTraq Venus on ttyUSB0 and sends some configuration commands

TODO:
- Verify device has booted after each configuration command has been sent
- Confirm setting with appropriate read command and parse response
- Decide how to actually configure things


Things we definitely want to configure:
    Navigation mode = Airborn
    Bitrate = 115200
    Binary mode (maybe?)
    Update rate = 20 Hz
    Disable SAEE

Venus 8 ROM versions:
    Kernel Version 2.0.2
    Software Version 1.8.27
    Revision 2013.2.21
"""

@contextmanager
def tcp(ip, port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind(('', 0))
    sock.connect((ip, port))
    yield sock
    sock.close()

def cmd(string):
    string = format_data(string)
    with tcp(b'10.10.10.40', 23) as sock:
        data = '#VNUS{}\r\n'.format(string).encode('ascii', 'ignore')
        sock.send(data)

def format_data(data):
    check = 0
    string = "A0A1{:04X}".format(len(data))
    for byte in data:
        check ^= byte
        string += "{:02X}".format(byte)
    string += "{:02X}0D0A".format(check & 0xFF)
    return string


cmd([0x05, 0x00, 0x05, 1])	# set baud 115200
#cmd([0x64, 0x17, 5, 1])	# Navigation mode = airborne
#cmd([0x63, 0x01, 0, 1])	# Disable SAEE
#cmd([0x0E, 20, 1])[30:]	# 20Hz
