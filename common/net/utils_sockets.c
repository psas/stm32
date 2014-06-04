#include <string.h>

#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "lwipthread.h"

#include "utils_sockets.h"

static int  recvSeqPacket(struct SeqSocket* ss, int packetLen);

static SeqErrorLogger   seqErrorLogger;

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

int recvSeqPacket(struct SeqSocket* ss, int packetLen) {
   uint32_t	seq;

   if (packetLen < 0)
      return packetLen;

   if (packetLen < (int)sizeof(uint32_t)) {
      if (seqErrorLogger)
         seqErrorLogger(SEQ_noseq, ss->seqRecv, 0, ss->buffer - sizeof(uint32_t), (size_t)packetLen);

      errno = EIO;
      return -1;
   }

   seq = ntohl(((uint32_t*)ss->buffer)[-1]);

   if (seq < ss->seqRecv) {
      if (seqErrorLogger)
         seqErrorLogger(SEQ_backward, ss->seqRecv, seq, ss->buffer, (size_t)packetLen - sizeof(uint32_t));

      ss->seqRecv = seq + 1;
      errno = EIO;
      return -1;
   } else if (seq > ss->seqRecv) {
      if (seqErrorLogger)
         seqErrorLogger(SEQ_skip, ss->seqRecv, seq, ss->buffer, (size_t)packetLen - sizeof(uint32_t));
   }

   ss->seqRecv = seq + 1;

   return packetLen - sizeof(uint32_t);
}

int seq_read(struct SeqSocket* ss) {
   return recvSeqPacket(ss, read(ss->socket, ss->buffer - sizeof(uint32_t), ss->maxSize + sizeof(uint32_t)));
}

int seq_recv(struct SeqSocket* ss, int flags) {
   return recvSeqPacket(ss, recv(ss->socket, ss->buffer - sizeof(uint32_t), ss->maxSize + sizeof(uint32_t), flags));
}

int seq_recvfrom(struct SeqSocket* ss, int flags, struct sockaddr* from, socklen_t* fromlen) {
   return recvSeqPacket(ss, recvfrom(ss->socket, ss->buffer - sizeof(uint32_t), ss->maxSize + sizeof(uint32_t), flags, from, fromlen));
}

int seq_send(struct SeqSocket* ss, size_t size, int flags) {
   int   s;

   if (size > ss->maxSize) {
      errno = EMSGSIZE;
      return -1;
   }

   ((uint32_t*)ss->buffer)[-1] = htonl(ss->seqSend);

   s = send(ss->socket, ss->buffer - sizeof(uint32_t), size + sizeof(uint32_t), flags);
   if (s < 0)
      return s;

   ++ss->seqSend;

   return s;
}

int seq_sendto(struct SeqSocket* ss, size_t size, int flags, const struct sockaddr* to, socklen_t tolen) {
   int   s;

   if (size > ss->maxSize) {
      errno = EMSGSIZE;
      return -1;
   }

   ((uint32_t*)ss->buffer)[-1] = htonl(ss->seqSend);

   s = sendto(ss->socket, ss->buffer - sizeof(uint32_t), size + sizeof(uint32_t), flags, to, tolen);
   if (s < 0)
      return s;

   ++ss->seqSend;

   return s;
}

void seq_set_error_logger(SeqErrorLogger logger) {
   seqErrorLogger = logger;
}

void seq_socket_init(struct SeqSocket* ss, int socket) {
   ss->socket = socket;
   ss->seqSend = 0;
   ss->seqRecv = 0;
}

int seq_write(struct SeqSocket* ss, size_t size) {
   int   s;

   if (size > ss->maxSize) {
      errno = EMSGSIZE;
      return -1;
   }

   ((uint32_t*)ss->buffer)[-1] = htonl(ss->seqSend);

   s = write(ss->socket, ss->buffer - sizeof(uint32_t), size + sizeof(uint32_t));
   if (s < 0)
      return s;

   ++ss->seqSend;

   return s;
}
