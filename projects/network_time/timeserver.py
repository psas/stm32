#!/usr/bin/python
import socket
import time
import math
from struct import *
from enum import Enum

class MsgType(Enum):
	t1 = 1
	t2 = 2

IP_ADDR = "10.0.0.2"
SERVER_LISTEN_PORT = 31337

sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
sock.bind((IP_ADDR, SERVER_LISTEN_PORT))

while True:
	data, addr = sock.recvfrom(136) #
	(msgType,t1Secs,t1MS,t2Secs,t2MS) = unpack("!cIIII",data)  #network byte order, char followed by 4 unsigned ints
	msgType = int(msgType,8) #make sure its treated as an int
	currenttime = time.time()
	print "received message:", data
	if msgType == MsgType.t1:
		respSock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		currenttime = time.time()
		(msecs,secs) = math.modf(currenttime)
		msecs = msecs * 1000
		response = pack("!cIIII",1,secs,msecs,0,0)
		respSock.sendto(response(addr,31338))
	elif msgType == MsgType.t2:
		respSock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		currenttime = time.time()
		(msecs,secs) = math.modf(currenttime)
		msecs = msecs * 1000
		response = pack("!cIIII",1,secs,msecs,0,0)
		respSock.sendto(response(addr,31338))
