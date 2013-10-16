#!/usr/bin/python
import struct

f = open("data_log.txt", "rb")
try:
    byte = f.read(1)
    while byte != "":
        # Do stuff with byte.
        print struct.unpack('B', byte[0])[0]
        byte = f.read(1)
finally:
    f.close()
