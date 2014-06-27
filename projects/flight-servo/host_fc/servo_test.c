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
#include "utils_sockets.h"

#define BUFLEN 64

static struct SeqSocket sock = DECL_SEQ_SOCKET(sizeof(RCOutput));

void sendpwm(int pulsewidth, int servodisable){
    uint8_t buffer[sizeof(RCOutput)];
    ssize_t send_status;
    RCOutput rc_packet;

    rc_packet.u16ServoPulseWidthBin14 = htons(pulsewidth);
    rc_packet.u8ServoDisableFlag = servodisable;
    memcpy(sock.buffer, &rc_packet, sizeof(RCOutput));

    send_status = seq_send(&sock, sizeof(RCOutput), 0);

    if (send_status < 0) {
        printf("local error while sending message! errno: %i\n", errno);
    } else {
        printf("successfully sent control message with pulsewidth %i\n", pulsewidth);
    }
}

int main(void){
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    assert(s != -1);
    struct sockaddr_in servo_ctl;
    set_sockaddr((struct sockaddr*)&servo_ctl, "10.0.0.30", 35003);

    if (connect(s, (struct sockaddr *)&servo_ctl, sizeof(servo_ctl))) {
        printf("Could not connect to servo board! errno: %i\n", errno);
        return errno;
    } else {
        printf("Connected to servo control board!\n");
    }

    seq_socket_init(&sock, s);

    struct timespec sleeptime;
    int pulsewidth;
    for(;;) {
        sleeptime.tv_sec   = 0;
        sleeptime.tv_nsec  = 25000000;  /* 25 mS */

        for (pulsewidth = 1100; pulsewidth <= 1900; pulsewidth += 10) {
            sendpwm(pulsewidth, 0);
            nanosleep(&sleeptime, NULL);
        }

        for (pulsewidth = 1900; pulsewidth >= 1100; pulsewidth -= 10) {
            sendpwm(pulsewidth, 0);
            nanosleep(&sleeptime, NULL);
        }

        sleeptime.tv_sec   = 4;
        sleeptime.tv_nsec  = 25000000;  /* 25 mS */
        sendpwm(1500, 1);
        nanosleep(&sleeptime, NULL);
    }
}

#if 0
    int16_t ticks = 0;
    while (TRUE) {
        uint16_t pos = PWM_CENTER;

        // calculate pos
        if (ticks < 10000) {
            // move from center out to either side and back over two seconds
            int16_t offset = 500 - abs(500 - (ticks % 1000));
            if (ticks % 2000 < 1000) {
                pos = PWM_CENTER + offset;
            } else {
                pos = PWM_CENTER - offset;
            }
        } else if (ticks < 20000) {
            // test speed controls by moving servo as fast as possible
            if (ticks % 2000 < 1000) {
                pos = 1000;
            } else {
                pos = 2000;
            }
        } else {
            ticks = -1;
        }

        PositionCommand* cmd = (PositionCommand*) chPoolAlloc(&pos_cmd_pool);
        if (cmd == NULL) continue;
        cmd->position = (uint16_t) pos;
        chMBPost(&servo_commands, (msg_t) cmd, TIME_IMMEDIATE);

        ticks++;
        chThdSleepMilliseconds(1);
    }
#endif

