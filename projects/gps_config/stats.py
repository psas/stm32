#!/usr/bin/python3
import struct
import statistics

if __name__ == "__main__":
	f = open('gpslog', 'rb')

	samples = []
	while True:
		data = f.read(2)
		if data == "" or len(data) != 2:
			break
		samples.append(struct.unpack('h', data)[0])
	print( "samples:", len(samples))
	print( "min:", min(samples), "max:", max(samples))
	print( "mean:", statistics.mean(samples))
	print( "median:", statistics.median(samples))
	print( "stdev:", statistics.stdev(samples))
