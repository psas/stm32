
/*
 * Socket and address related utilities
 *
 */

#ifndef UTILS_SOCKETS_H_
#define UTILS_SOCKETS_H_

#include "lwipthread.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"

/* Ethernet MTU in bytes - useful for creating UDP rx and tx buffers*/
#define ETH_MTU 1500

/* Fills out an lwipthread_opts struct */
void set_lwipthread_opts(struct lwipthread_opts * ip_opts,
    const char * ip, const char * netmask, const char * gateway, uint8_t * mac);

/* Starts the lwip core thread with ip_opts */
void lwipThreadStart(struct lwipthread_opts * ip_opts);


/* Fills out a sockaddr_in with AF_INET, ip, and port in network byteorder */
void set_sockaddr(struct sockaddr * addr, const char * ip, uint16_t port);

/* Returns an AF_INET UDP socket bound to addr, or less than 0 on failure */
int get_udp_socket(const struct sockaddr *addr);

struct SeqSocket {
    size_t maxSize;
    int socket;
    uint32_t seqSend;
    uint32_t seqRecv;
    uint8_t* buffer;
};

#define DECL_SEQ_SOCKET(MAXSIZE) { MAXSIZE, 0, 0, 0, (((uint8_t[MAXSIZE + sizeof(uint32_t)]){ 0 }) + sizeof(uint32_t)) }

typedef enum SeqError {
    SEQ_noseq,
    SEQ_backward,
    SEQ_skip,
} SeqError;

typedef int (*SeqErrorLogger)(SeqError error, uint32_t expSeq, uint32_t rcvdSeq, const void* data, size_t size);

void    seq_set_error_logger(SeqErrorLogger logger);

void    seq_socket_init(struct SeqSocket* ss, int socket);

int     seq_recv(struct SeqSocket* ss, int flags);
int     seq_send(struct SeqSocket* ss, size_t size, int flags);

int     seq_recvfrom(struct SeqSocket* ss, int flags, struct sockaddr* from, socklen_t* fromlen);
int     seq_sendto(struct SeqSocket* ss, size_t size, int flags, const struct sockaddr* to, socklen_t tolen);

int     seq_read(struct SeqSocket* ss);
int     seq_write(struct SeqSocket* ss, size_t size);

#endif
