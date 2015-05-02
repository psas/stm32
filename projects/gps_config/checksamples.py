#!/usr/bin/python3

import csv
from pprint import pprint
import struct

if __name__ == '__main__':
    counter = 0
    with open('gpslog', 'rb') as gpslog:
        while True:
            raw = gpslog.read(1)
            if len(raw) != 1:
                break
            data = struct.unpack('B', raw)[0]
            print(data, end=" ")
            counter += 1
            if counter > 1023:
                print('\n')
                counter = 0

