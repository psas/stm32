/*! \file enet_udp_fc.c
 */

/*! Experiment with host side capture of udp messages */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define         BUFLEN          512
#define         NPACK           100
#define         PORT            35000
#define         OUTPORT         35003
#define         SENSOR_IP       "192.168.0.196"
#define         CONTROL_IP      "192.168.0.197"

static void die_nice(char *s) {
    perror(s);
    exit(1);
}

int main(void) {
    struct      sockaddr_in     si_me,  si_other;
    int         s, i;
    int         slen=sizeof(si_other);
    char        buf[BUFLEN];

    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        die_nice("socket");

    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family       = AF_INET;
    si_me.sin_port         = htons(PORT);
    si_me.sin_addr.s_addr  = htonl(INADDR_ANY);

    if (bind(s, (struct sockaddr *)&si_me, (socklen_t)  sizeof(si_me))==-1)
        die_nice("bind");

    si_other.sin_port =  htons(PORT);
    for (i=0; i<NPACK; i++) {
        if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *)&si_other, &slen)==-1)
            die_nice("recvfrom()");
        printf("Received packet from %s:%d\nData: %s\n\n",
                inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);
    }

    close(s);
    return 0;
}

