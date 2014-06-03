#!/usr/bin/env python

import socket
import time

#UDP_IP = "10.0.0.40"
UDP_IP = "10.0.0.45"
UDP_PORT = 36201
MESSAGE = "Hello"

print ("UDP target IP:", UDP_IP)
print ("UDP target port:", UDP_PORT)
print ("message:", MESSAGE)

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP
while True:
        print (".")
        sock.sendto(MESSAGE, (UDP_IP, UDP_PORT))
        time.sleep(1)



