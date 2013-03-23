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

#define         BUFLEN          10
#define         NPACK           5
#define         OUTPORT         35003
#define         SRV_IP          "192.168.0.196"

void die_nice(char *s) {
    perror(s);
    exit(1);
}

int main(void) {
  struct sockaddr_in si_other;
  int                s, i, slen=sizeof(si_other);
  char               buf[BUFLEN];

  if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1) {
    die_nice("socket");
  }

  memset((char *) &si_other, 0, sizeof(si_other));
  si_other.sin_family = AF_INET;
  si_other.sin_port = htons(OUTPORT);
  if (inet_aton(SRV_IP, &si_other.sin_addr)==0) {
    fprintf(stderr, "inet_aton() failed\n");
    exit(1);
  }

  for (i=0; i<NPACK; i++) {
    printf("Sending packet %d\n", i);
    sprintf(buf, "%s", "Shinybit\n");
    if (sendto(s, buf, BUFLEN, 0,(struct sockaddr *) &si_other, slen)==-1) {
      die_nice("sendto()");
    }
    printf("Sent packet to %s:%d\nData: %s\n\n",
                inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);
 
    sleep(1);
  }

  close(s);
  return 0;
}
