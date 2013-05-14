
CC=gcc
CFLAGS += -I../../../devices/include -I../../net_common -lpthread -g
LDFLAGS+= -lpthread

.PHONY: clean

all: mpu_fc

datap_fc: mpu_fc.c

clean:
	$(RM) mpu_fc

