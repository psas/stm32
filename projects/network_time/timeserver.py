#!/usr/bin/python
import socket
import time
import math
from struct import *
from enum import enum

class MsgType(Enum):
	t1 = 1
	t2 = 2

IP_ADDR = "127.0.0.1"
SERVER_LISTEN_PORT = 31337

sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
sock.bind((IP_ADDR, SERVER_LISTEN_PORT))

while True:
	data, addr = sock.recvfrom(1024) #
	currenttime = time.time()
	(msecs,secs) = math.modf(currenttime)
	msecs = msecs * 1000
	print "received message:", data
	print "packet received at:", msecs, secs
