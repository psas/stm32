
CC=gcc
CFLAGS += -lpthread -g

.PHONY: clean

all: datap_fc

datap_fc: datap_fc.c

eth_udp_fc: eth_udp_fc.c

eth_fc_to_sensorboard: eth_fc_to_sensorboard.c

clean:
	$(RM) eth_udp_fc eth_fc_to_sensorboard datap_fc

