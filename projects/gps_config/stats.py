#!/usr/bin/python3
import struct
import statistics

if __name__ == "__main__":
    f = open('data/1.95mhz', 'rb')

    samples = []
    total = 0
    i0 = i1 = q0 = q1 = 0
    while True:
        data = f.read(1)
        if data == "" or len(data) != 1:
            break
        sample = struct.unpack('B', data)[0]

        if sample & (1 << 0):
            i1 += 1
        if sample & (1 << 1):
            i0 += 1
        if sample & (1 << 2):
            q1 += 1
        if sample & (1 << 3):
            q0 += 1
        if sample & (1 << 4):
            i1 += 1
        if sample & (1 << 5):
            i0 += 1
        if sample & (1 << 6):
            q1 += 1
        if sample & (1 << 7):
            q0 += 1

        total += 2


    print('i0:{:%} i1:{:%} q0:{:%} q1:{:%}'.format(i0/total, i1/total, q0/total, q1/total))
