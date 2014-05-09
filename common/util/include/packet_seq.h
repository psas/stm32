#ifndef __packet_seq_h
#define __packet_seq_h

typedef enum SeqError {
	SEQ_backward,
	SEQ_datasize,
	SEQ_skip,
} SeqError;

typedef int (*SeqErrorHandler)(SeqError error, unsigned long expSeq, unsigned long rcvdSeq, const void* data, size_t size);

#define SEQ_SOCKET_MAX_SIZE		2048
#define SEQ_SOCKET_MAX_SOCKETS	10

void seq_set_error_handler(SeqErrorHandler handler);

int seq_socket(int domain, int type, int protocol);
int seq_socket_wrap(int s);
int seq_get_fd(int ss);
int seq_close(int ss);

int seq_recv(int ss, void* mem, size_t len, int flags);
int seq_read(int ss, void* mem, size_t len);
int seq_recvfrom(int ss, void* mem, size_t len, int flags, struct sockaddr* from, socklen_t* fromlen);
int seq_send(int ss, const void* dataptr, size_t size, int flags);
int seq_sendto(int ss, const void* dataptr, size_t size, int flags, const struct sockaddr* to, socklen_t tolen);
int seq_write(int ss, const void* dataptr, size_t size);

int get_seq_socket(const struct sockaddr* addr);

#endif
