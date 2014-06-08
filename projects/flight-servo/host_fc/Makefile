
CC=gcc
CFLAGS += -I.. -g 
LDFLAGS+= 

.PHONY: clean

all: servo_test

servo_test: servo_test.o utils_sockets.o

clean:
	$(RM) servo_test servo_test.o utils_sockets.o

