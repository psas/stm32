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
	#(msgType,t1Secs,t1MS,t2Secs,t2MS) = unpack("<HIIII",data)  #network byte order, char followed by 4 unsigned ints
	data,addr = sock.recvfrom(68);
	(msgType,t1_secs,t1_ms,t2_secs,t2_ms) = unpack("<50xHIIII",data)
	print msgType, t1_secs, t1_ms, t2_secs, t2_ms,  "\n"
	currenttime = time.time()
	if msgType == MsgType.t1:
		print "sending type 1 message"
		respSock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		currenttime = time.time()
		(msecs,secs) = math.modf(currenttime)
		msecs = msecs * 1000
		response = pack("<HIIII",1,secs,msecs,0,0)
		respSock.sendto(response,(addr[0],31338))
		print "Sent message type 1 secs %d msecs %d." % (secs,msecs)
	elif msgType == MsgType.t2:
		respSock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		currenttime = time.time()
		(msecs,secs) = math.modf(currenttime)
		msecs = msecs * 1000
		response = pack("<HIIII",1,secs,msecs,0,0)
		respSock.sendto(response,(addr[0],31338))
