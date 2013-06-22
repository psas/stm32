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

#include <math.h>

#define BOARD_IP "192.168.0.197"
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

/*! \brief Convert ms value to uint16 shifted by 2^14 */
static uint16_t ms_to_pkt(double ms) {
    double   shifted = ms * pow(2, 14);
    uint16_t retval  = (uint16_t) shifted;

    return retval;
}


int main(void) {
    struct timespec         sleeptime;

    double                  msecs     = 0.0;

    struct sockaddr_in      si_other;

    char                    buf[sizeof(PWM_packet) - 1];
    int                     s, i, j;
    int                     slen      = sizeof(si_other);

	RC_OUTPUT_STRUCT_TYPE rc_packet;

	msecs                             = 1.5 * pow(2, 14);
	rc_packet.u16ServoPulseWidthBin14 = (uint16_t) msecs;
	rc_packet.u8ServoDisableFlag      = 0;

	s                   = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	assert(s != -1);
	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port   = htons(PORT);
	if (inet_aton(BOARD_IP, &si_other.sin_addr)==0) {
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}
	sleeptime.tv_sec   = 0;
	sleeptime.tv_nsec  = 50000;  /* 50 uS */
	for (;;) {
		double pulsewidth;
		for (pulsewidth = 1.000; pulsewidth < 2.000; pulsewidth += 0.050) {
			rc_packet.u16ServoPulseWidthBin14 = ms_to_pkt(pulsewidth);
			rc_packet.u8ServoDisableFlag      = 0;

			memcpy(&buf,&rc_packet,sizeof(RC_OUTPUT_STRUCT_TYPE));
			j = sendto(s,buf,sizeof(PWM_packet),0,(const struct sockaddr *)&si_other, slen);
			assert(j != -1);

			nanosleep(&sleeptime, NULL);
		}
	}
}
