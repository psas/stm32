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
#include <errno.h>

#include "psas_packet.h"

#define BUFLEN 64

void set_sockaddr(struct sockaddr_in * addr, const char * ip, int port){
    //Create an address (remember to have the data in network byte order)
    memset(addr, 0, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET,
    addr->sin_port = htons(port);
    inet_aton(ip, &addr->sin_addr);
}

void sendpwm(int s, int pulsewidth, int servodisable){
    static uint64_t sequence_no = 0;
    uint8_t buffer[sizeof(RCOutput)];
    ssize_t send_status;
    RCOutput rc_packet;

    rc_packet.time = sequence_no++;
    rc_packet.u16ServoPulseWidthBin14 = pulsewidth;
    rc_packet.u8ServoDisableFlag      = servodisable;
    memcpy(buffer, &rc_packet, sizeof(RCOutput));

    if ((send_status = send(s, buffer, sizeof(RCOutput), 0)) < 0) {
        printf("local error while sending message! errno: %i\n", errno);
    } else {
        printf("successfully sent control message with pulsewidth %i\n", pulsewidth);
    }
}

int main(void){
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    assert(s != -1);
    struct sockaddr_in servo_ctl;
    set_sockaddr(&servo_ctl, "10.0.0.30", 35003);

    if (connect(s, (struct sockaddr *)&servo_ctl, sizeof(servo_ctl))) {
        printf("Could not connect to servo board! errno: %i\n", errno);
        return errno;
    } else {
        printf("Connected to servo control board!\n");
    }

    struct timespec sleeptime;
    int pulsewidth;
    for(;;) {
        sleeptime.tv_sec   = 0;
        sleeptime.tv_nsec  = 25000000;  /* 25 mS */

        for (pulsewidth = 1100; pulsewidth <= 1900; pulsewidth += 10) {
            sendpwm(s, pulsewidth, 0);
            nanosleep(&sleeptime, NULL);
        }

        for (pulsewidth = 1900; pulsewidth >= 1100; pulsewidth -= 10) {
            sendpwm(s, pulsewidth, 0);
            nanosleep(&sleeptime, NULL);
        }

        sleeptime.tv_sec   = 4;
        sleeptime.tv_nsec  = 25000000;  /* 25 mS */
        sendpwm(s, 1500, 1);
        nanosleep(&sleeptime, NULL);
    }
}
