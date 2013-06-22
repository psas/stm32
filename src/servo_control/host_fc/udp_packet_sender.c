#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <inttypes.h>
#include <assert.h>
#define BOARD_IP "10.0.0.2"
#define PORT 35003
#define BUFLEN 64

struct RC_OUTPUT_STRUCT_TYPE {
    // Servo ON-Time in milliseconds x 2^14
    // Example: 1.5 msec = 1.5 x 2^14 = 24576
    uint16_t u16ServoPulseWidthBin14;

    // Disable servo (turn off PWM) when this flag is not 0
    uint8_t u8ServoDisableFlag;
} __attribute__((packed)) ;
typedef struct RC_OUTPUT_STRUCT_TYPE RC_OUTPUT_STRUCT_TYPE;

struct PWM_packet {
    char                ID[4];
    uint8_t             timestamp[6];
    uint16_t            data_length;
    uint16_t            duty_cycle;
} __attribute__((packed)) ;
typedef struct PWM_packet PWM_packet;

int main(void) {
	PWM_packet test_packet;
	const char	myid[(sizeof("PWDC")-1)] = "PWDC";
	const char	mytime[(sizeof("000000")-1)] = "000000";
	strncpy(test_packet.ID, myid, sizeof(myid));
	strncpy(test_packet.timestamp, mytime, sizeof(mytime));
	struct sockaddr_in si_other;
	char buf[sizeof(PWM_packet) - 1];
	int s, i, slen=sizeof(si_other);
	int j;
	s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	assert(s != -1);
	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
	if (inet_aton(BOARD_IP, &si_other.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}
	for (;;) {
		int duty;
		for (duty = 16384; duty < 65535; duty += 16384) {
			test_packet.duty_cycle = duty;
			memcpy(&buf,&test_packet,sizeof(PWM_packet));
			j = sendto(s,buf,sizeof(PWM_packet),0,(const struct sockaddr *)&si_other, slen);
			assert(j != -1);
			sleep(1);
		}
	}
}
