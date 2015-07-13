#!/usr/bin/python
from contextlib import contextmanager
import socket
import sys
import time

"""
Connects to a SkyTraq Venus on ttyUSB0 and sends some configuration commands

TODO:
- Verify device has booted after each configuration command has been sent
- Confirm setting with appropriate read command and parse response
- Decide how to actually configure things


Things we definitely want to configure:
    Navigation mode = Airborn
    Bitrate = 115200
    Binary mode
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

def baud_rate(code):
    return [4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600][code]

def meas_data_rate(code):
    """Update rate for binary measurement data output. See SkyTraq AN0030."""
    return [1, 2, 4, 5, 10, 20][code]

max_channels = 20 # we don't expect more than this many satellites locked at once

baud_rate_code = 6 # see baud_rate()
nav_data_rate = 50 # Hz; update rate for navigation data message (see AN0028)
meas_data_rate_code = 5 # see meas_data_rate()
meas_time_enabled = 1
raw_meas_enabled = 1
sv_ch_status_enabled = 1
rcv_state_enabled = 1

bytes_per_second = nav_data_rate * 66

if meas_time_enabled:
    bytes_per_second += meas_data_rate(meas_data_rate_code) * 17

if raw_meas_enabled:
    bytes_per_second += meas_data_rate(meas_data_rate_code) * (10 + 23 * max_channels)

if sv_ch_status_enabled:
    bytes_per_second += meas_data_rate(meas_data_rate_code) * (10 + 10 * max_channels)

if rcv_state_enabled:
    # this message is always sent at 1Hz
    bytes_per_second += 88

# GPS subframe messages
bytes_per_second += 40 * max_channels / 6

print "minimum baud rate: {}".format(bytes_per_second * 10)
print "selected baud rate: {}".format(baud_rate(baud_rate_code))

if bytes_per_second * 10 > baud_rate(baud_rate_code):
    print " *** can't keep up with this config at this baud rate!"
    sys.exit(1)

if False:
    print "re-setting baud rate; after this, reprogram the STM32 with the new baud rate and reboot the board"
    cmd([0x05, 0x00, baud_rate_code, 1])
    sys.exit(0)

cmd([0x09, 0x02, 1])	# Set binary output mode
cmd([0x64, 0x17, 5, 1])	# Navigation mode = airborne
cmd([0x63, 0x01, 2, 1])	# Disable "Self-Aided Ephemeris Estimation" (see http://navspark.mybigcommerce.com/content/GNSSViewerUserGuide.pdf)
cmd([0x0E, nav_data_rate, 1])	# System position rate
cmd([0x11, nav_data_rate, 1])	# Navigation data message rate
cmd([0x1E, meas_data_rate_code, meas_time_enabled, raw_meas_enabled, sv_ch_status_enabled, rcv_state_enabled, 0b0001, 1])	# Binary measurement data enable and rate
