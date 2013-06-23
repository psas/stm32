
CC=gcc
CFLAGS += -I.. -lpthread -g
LDFLAGS+= -lpthread

.PHONY: clean

all: udp_packet_sender

udp_packet_sender: udp_packet_sender.c

clean:
	$(RM) udp_packet_sender

