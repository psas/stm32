#include <string.h>

#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "lwipthread.h"

#include "utils_sockets.h"

void set_lwipthread_opts(struct lwipthread_opts * ip_opts,
    const char * ip, const char * netmask, const char * gateway, uint8_t * mac)
{
    ip_opts->macaddress = mac;
    ip_opts->address = inet_addr(ip);
    ip_opts->netmask = inet_addr(netmask);
    ip_opts->gateway = inet_addr(gateway);
}

void lwipThreadStart(struct lwipthread_opts * ip_opts){
    chThdCreateStatic(wa_lwip_thread,
                      sizeof(wa_lwip_thread),
                      NORMALPRIO + 2,
                      lwip_thread,
                      ip_opts);
}

void set_sockaddr(struct sockaddr * addr, const char * ip, uint16_t port){
    //Create an address (remember to have the data in network byte order)
    struct sockaddr_in * in = (struct sockaddr_in *)addr;
    memset(in, 0, sizeof(struct sockaddr_in));
    in->sin_family = AF_INET,
    in->sin_port = htons(port);
    inet_aton(ip, &in->sin_addr);
}


int get_udp_socket(const struct sockaddr *addr){
    //Create the socket
    int s = socket(AF_INET,  SOCK_DGRAM, 0);
    if(s < 0){
        /* socket allocation failure */
        return -1;
    }

    //bind our own address to the socket
    if(bind(s, addr, sizeof(struct sockaddr_in)) < 0){
        /* socket bind failure */
        return -2;
    }
    return s;
}


/* Sequenced socket utilities */

static SeqErrorLogger seqErrorLogger;

void seqSetErrorLogger(SeqErrorLogger logger) {
   seqErrorLogger = logger;
}

int seqSocket(struct SeqSocket* ss, const struct sockaddr * addr) {
    ss->socket = get_udp_socket(addr);
    if(ss->socket < 0)
        return ss->socket;
    return 0;
}

int seqRecvfrom(struct SeqSocket* ss, int flags, struct sockaddr* from, socklen_t* fromlen) {
   int packetLen = recvfrom(ss->socket, ss->buffer - sizeof(uint32_t), ss->maxSize + sizeof(uint32_t), flags, from, fromlen);
   if (packetLen < 0)
      return packetLen;
   unsigned int len = packetLen;

   if (len < sizeof(uint32_t)) {
      if (seqErrorLogger)
         seqErrorLogger(0, 0, ss->buffer - sizeof(uint32_t), len);
      errno = EIO;
      return -1;
   }

   uint32_t seq = ntohl(((uint32_t*)ss->buffer)[-1]);
   if (seq != ss->seqRecv)
      if (seqErrorLogger)
         seqErrorLogger(ss->seqRecv, seq, ss->buffer, len - sizeof(uint32_t));

   ss->seqRecv = seq + 1;
   if(seq < ss->seqRecv) {
      errno = EIO;
      return -1;
   } else {
      return packetLen - sizeof(uint32_t);
   }
}

int seqRecv(struct SeqSocket* ss, int flags) {
   return seqRecvfrom(ss, flags, NULL, NULL);
}

int seqRead(struct SeqSocket* ss) {
   return seqRecvfrom(ss, 0, NULL, NULL);
}

int seqSendto(struct SeqSocket* ss, size_t size, int flags, const struct sockaddr* to, socklen_t tolen) {
   if (size > ss->maxSize) {
      errno = EMSGSIZE;
      return -1;
   }

   ((uint32_t*)ss->buffer)[-1] = htonl(ss->seqSend);
   int ret = sendto(ss->socket, ss->buffer - sizeof(uint32_t), size + sizeof(uint32_t), flags, to, tolen);
   if (ret < 0)
      return ret;

   ++ss->seqSend;
   return ret;
}

int seqSend(struct SeqSocket* ss, size_t size, int flags) {
    if (size > ss->maxSize) {
      errno = EMSGSIZE;
      return -1;
   }

   ((uint32_t*)ss->buffer)[-1] = htonl(ss->seqSend);
   int ret = send(ss->socket, ss->buffer - sizeof(uint32_t), size + sizeof(uint32_t), flags);
   if (ret < 0)
      return ret;

   ++ss->seqSend;
   return ret;
}

int seqWrite(struct SeqSocket* ss, size_t size) {
    return seqSend(ss, size, 0);
}




