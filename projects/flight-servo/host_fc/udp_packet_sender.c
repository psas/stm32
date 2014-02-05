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

#include "psas_packet.h"

#define BUFLEN 64

void set_sockaddr(struct sockaddr_in * addr, const char * ip, int port){
    //Create an address (remember to have the data in network byte order)
    memset(addr, 0, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET,
    addr->sin_port = htons(port);
    inet_aton(ip, &addr->sin_addr);
}

void sendpwm(int s, int pulsewidth, int servodisable, struct timespec * sleep){
    RCOutput rc_packet;
    uint8_t buf[sizeof(RCOutput)];
    printf("pulsewidth: %u\n", pulsewidth);
    rc_packet.u16ServoPulseWidthBin14 = pulsewidth;
    rc_packet.u8ServoDisableFlag      = servodisable;
    memcpy(buf, &rc_packet, sizeof(RCOutput));
//          printf("sizeof: %x\n", sizeof(RC_OUTPUT_STRUCT_TYPE));
//          printf("buf:   %x %x %x\n", buf[0], buf[1], buf[2]);
//            printf("rc:    %x %x %x\n", (rc_packet.u16ServoPulseWidthBin14&0xff00)>>8, rc_packet.u16ServoPulseWidthBin14&0x00ff , rc_packet.u8ServoDisableFlag);
//            printf("val:   %x\n\n", rc_packet.u16ServoPulseWidthBin14);
    write(s, buf, sizeof(RCOutput));
    nanosleep(sleep, NULL);
}

int main(void){

    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    assert(s != -1);
    struct sockaddr_in servo_ctl;
    set_sockaddr(&servo_ctl, "10.0.0.30", 35003);
    connect(s, (struct sockaddr *)&servo_ctl, sizeof(servo_ctl));

    struct timespec sleeptime;
    int pulsewidth;
    for(;;) {
        sleeptime.tv_sec   = 0;
        sleeptime.tv_nsec  = 25000000;  /* 25 mS */
        for (pulsewidth = 1050; pulsewidth < 1950; pulsewidth += 10) {
            sendpwm(s, pulsewidth, 0, &sleeptime);
        }
        for (pulsewidth = 1950; pulsewidth > 1050; pulsewidth -= 10) {
            sendpwm(s, pulsewidth, 0, &sleeptime);
        }

        sleeptime.tv_sec   = 4;
        sleeptime.tv_nsec  = 25000000;  /* 25 mS */
        sendpwm(s, 1500, 1, &sleeptime);
    }
}
