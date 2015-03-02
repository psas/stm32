#!/usr/bin/python3

import csv
from pprint import pprint
import struct

if __name__ == '__main__':
    with open('gpslogic.csv') as csvfile:
        reader = csv.reader(csvfile)
        print(next(reader))
        prev = next(reader)
        convert = []
        samples = []
        idx = 0
        skip = 40
        for row in reader:
            if row[2] == prev[2]:
                prev = row
                continue

            if row[2].strip() == '0' and prev[2].strip() == '1':
                if idx < skip:
                    skip -= 1
                    continue

                if row[1] != prev[1]:
                    print("Bad data")
                    break

                convert.append(row[1].strip())
                if idx % 16 == 0 and idx != 0:
                    samples.append(int(''.join(convert), 2))
                    convert = []

                if idx == 16*100:
                    break
                idx += 1

            prev = row

    es = []
    with open('gpslog', 'rb') as gpslog:

        sync = 2
        syncmask = 2**sync-1
        carry = 0
        while True:
            raw = gpslog.read(1)
            if len(raw) != 1:
                break
            data = struct.unpack('B', raw)[0]
            tmpcarry = data & syncmask
            data >>= sync
            data |= carry << 8-sync
            carry = tmpcarry
            es.append(data)


    pprint(es)


