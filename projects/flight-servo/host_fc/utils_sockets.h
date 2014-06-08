
/*
 * Socket and address related utilities
 *
 */

#ifndef UTILS_SOCKETS_H_
#define UTILS_SOCKETS_H_

#include <sys/socket.h>
#include <netinet/in.h>

typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;

/* Ethernet MTU in bytes - useful for creating UDP rx and tx buffers*/
#define ETH_MTU 1500

/* Fills out a sockaddr_in with AF_INET, ip, and port in network byteorder */
void set_sockaddr(struct sockaddr * addr, const char * ip, uint16_t port);

/* Returns an AF_INET UDP socket bound to addr, or less than 0 on failure */
int get_udp_socket(const struct sockaddr *addr);

struct SeqSocket {
   size_t         maxSize;
   int            socket;
   unsigned long  seqSend;
   unsigned long  seqRecv;
   uint8_t*       buffer;
};

#define DECL_SEQ_SOCKET(MAXSIZE) { MAXSIZE, 0, 0, 0, (((uint8_t[MAXSIZE + sizeof(unsigned long)]){ 0 }) + sizeof(unsigned long)) }

typedef enum SeqError {
   SEQ_backward,
   SEQ_skip,
} SeqError;

typedef int (*SeqErrorLogger)(SeqError error, unsigned long expSeq, unsigned long rcvdSeq, const void* data, size_t size);

void  seq_set_error_logger(SeqErrorLogger logger);

void  seq_socket_init(struct SeqSocket* ss, int socket);

int   seq_recv(struct SeqSocket* ss, int flags);
int   seq_send(struct SeqSocket* ss, size_t size, int flags);

int   seq_recvfrom(struct SeqSocket* ss, int flags, struct sockaddr* from, socklen_t* fromlen);
int   seq_sendto(struct SeqSocket* ss, size_t size, int flags, const struct sockaddr* to, socklen_t tolen);

int   seq_read(struct SeqSocket* ss);
int   seq_write(struct SeqSocket* ss, size_t size);

#endif
