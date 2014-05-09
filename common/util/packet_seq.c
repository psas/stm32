#include <stdio.h>
#include <string.h>
#include "lwip/sockets.h"
#include "packet_seq.h"

/*
	seqSockets is the storage for all sequenced sockets.  The first unallocated
	socket is seqSockets[free_socket].  If free_socket < 0, then there are no
	sockets available.  Free sockets are chained in a singly linked list, using
	the socket field, which contains the index within seqSockets of the next
	free socket, or -1 indicating end of list.
*/

#define assert(e)	((void)(!!(e)))

typedef struct SeqSocket {
	int					socket;
	unsigned long		sendSeq;
	unsigned long		recvSeq;
} SeqSocket;

static int			default_error_handler(SeqError error, unsigned long expSeq, unsigned long rcvdSeq, const void* data, size_t size);
static void			init(void);
static int			recv_packet(int ss, void* mem, size_t size);

#ifdef NDEBUG
 #define validate_ss(ss) ((void)0)
#else
 static void		validate_ss(int ss);
#endif

static SeqErrorHandler	error_handler;
static int					free_socket;
static char					packetBuffer[SEQ_SOCKET_MAX_SIZE + sizeof(unsigned long)];
static SeqSocket			seqSockets[SEQ_SOCKET_MAX_SOCKETS];

int default_error_handler(SeqError error, unsigned long expSeq, unsigned long rcvdSeq, const void* data, size_t size) {
	(void)error;
	(void)expSeq;
	(void)rcvdSeq;
	(void)data;
	(void)size;

	return 0;
}

int get_seq_socket(const struct sockaddr* addr) {
	int	s = seq_socket(AF_INET, SOCK_DGRAM, 0);

	if (s < 0)
		return -1;

	if (lwip_bind(s, addr, sizeof(struct sockaddr)) < 0)
		return -2;

	return s;
}

void init(void) {
	int	i;

	for (i = 0 ; i < SEQ_SOCKET_MAX_SOCKETS ; ++i)
		if (i == SEQ_SOCKET_MAX_SOCKETS - 1)
			seqSockets[i].socket = -1;
		else
			seqSockets[i].socket = i + 1;
	free_socket = 0;

	error_handler = default_error_handler;
}

int recv_packet(int ss, void* mem, size_t size) {
	SeqSocket*		s;
	unsigned long	seq;
	int				ok;

	s = seqSockets + ss;

	if (size < sizeof(unsigned long)) {
		errno = EIO;
		return -1;
	}

	seq = ntohl(*(unsigned long*)packetBuffer);

	if (seq < s->recvSeq)
		ok = error_handler(SEQ_backward, s->recvSeq, seq, packetBuffer + sizeof(unsigned long), size - sizeof(unsigned long));
	else if (seq > s->recvSeq)
		ok = error_handler(SEQ_skip, s->recvSeq, seq, packetBuffer + sizeof(unsigned long), size - sizeof(unsigned long));
	else
		ok = 1;

	s->recvSeq = seq + 1;

	if (ok) {
		memcpy(mem, packetBuffer + sizeof(unsigned long), size - sizeof(unsigned long));
		return size - sizeof(unsigned long);
	} else {
		errno = EIO;
		return -1;
	}
}

int seq_close(int ss) {
	int	s;

	assert(error_handler);

	validate_ss(ss);

	s = seqSockets[ss].socket;

	seqSockets[ss].socket = free_socket;
	free_socket = ss;

	return lwip_close(s);
}

int seq_get_fd(int ss) {
	assert(error_handler);

	validate_ss(ss);

	return seqSockets[ss].socket;
}

int seq_read(int ss, void* mem, size_t len) {
	int	r;

	assert(error_handler);

	validate_ss(ss);

	if (len + sizeof(unsigned long) > SEQ_SOCKET_MAX_SIZE) {
		errno = ENOMEM;
		return -1;
	}

	r = lwip_read(seqSockets[ss].socket, packetBuffer, len + sizeof(unsigned long));
	if (r < 0)
		return r;

	return recv_packet(ss, mem, r);
}

int seq_recv(int ss, void* mem, size_t len, int flags) {
	int	r;

	assert(error_handler);

	validate_ss(ss);

	if (len > SEQ_SOCKET_MAX_SIZE) {
		errno = ENOMEM;
		return -1;
	}

	r = lwip_recv(seqSockets[ss].socket, packetBuffer, len + sizeof(unsigned long), flags);
	if (r < 0)
		return r;

	return recv_packet(ss, mem, r);
}

int seq_recvfrom(int ss, void* mem, size_t len, int flags, struct sockaddr* from, socklen_t* fromlen) {
	int	r;

	assert(error_handler);

	validate_ss(ss);

	if (len + sizeof(unsigned long) > SEQ_SOCKET_MAX_SIZE) {
		errno = ENOMEM;
		return -1;
	}

	r = lwip_recvfrom(seqSockets[ss].socket, packetBuffer, len + sizeof(unsigned long), flags, from, fromlen);
	if (r < 0)
		return r;

	return recv_packet(ss, mem, r);
}

int seq_send(int ss, const void* dataptr, size_t size, int flags) {
	SeqSocket*	s;

	assert(error_handler);

	validate_ss(ss);

	s = seqSockets + ss;

	if (size + sizeof(unsigned long) > SEQ_SOCKET_MAX_SIZE) {
		error_handler(SEQ_datasize, 0, 0, dataptr, size);
		errno = ENOMEM;
		return -1;
	}

	*(unsigned long*)packetBuffer = htonl(s->sendSeq++);
	memcpy(packetBuffer + sizeof(unsigned long), dataptr, size);

	return lwip_send(s->socket, packetBuffer, size + sizeof(unsigned long), flags);
}

int seq_sendto(int ss, const void* dataptr, size_t size, int flags, const struct sockaddr* to, socklen_t tolen) {
	SeqSocket*	s;

	assert(error_handler);

	validate_ss(ss);

	s = seqSockets + ss;

	if (size + sizeof(unsigned long) > SEQ_SOCKET_MAX_SIZE) {
		error_handler(SEQ_datasize, 0, 0, dataptr, size);
		errno = ENOMEM;
		return -1;
	}

	*(unsigned long*)packetBuffer = htonl(s->sendSeq++);
	memcpy(packetBuffer + sizeof(unsigned long), dataptr, size);

	return lwip_sendto(s->socket, packetBuffer, size + sizeof(unsigned long), flags, to, tolen);
}

void seq_set_error_handler(SeqErrorHandler handler) {
	assert(handler);

	if (!error_handler)
		init();

	error_handler = handler;
}

int seq_socket(int domain, int type, int protocol) {
	int	s, ss;

	s = socket(domain, type, protocol);
	if (s < 0)
		return s;

	ss = seq_socket_wrap(s);
	if (ss < 0) {
		lwip_close(s);
	}

	return ss;
}

int seq_socket_wrap(int s) {
	int	ss;

	if (!error_handler)
		init();

	if (free_socket < 0) {
		errno = ENFILE;
		return -1;
	}

	assert(free_socket >= 0 && free_socket < SEQ_SOCKET_MAX_SOCKETS);

	ss = free_socket;
	free_socket = seqSockets[free_socket].socket;

	seqSockets[ss].socket = s;
	seqSockets[ss].sendSeq = 0;
	seqSockets[ss].recvSeq = 0;

	return ss;
}

int seq_write(int ss, const void* dataptr, size_t size) {
	SeqSocket*	s;

	assert(error_handler);

	validate_ss(ss);

	s = seqSockets + ss;

	if (size + sizeof(unsigned long) > SEQ_SOCKET_MAX_SIZE) {
		error_handler(SEQ_datasize, 0, 0, dataptr, size);
		errno = ENOMEM;
		return -1;
	}

	*(unsigned long*)packetBuffer = htonl(s->sendSeq++);
	memcpy(packetBuffer + sizeof(unsigned long), dataptr, size);

	return lwip_write(s->socket, packetBuffer, size + sizeof(unsigned long));
}

#ifndef NDEBUG

void validate_ss(int ss) {
	int	count, i;

	assert(ss >= 0 && ss < SEQ_SOCKET_MAX_SOCKETS);

	count = 0;
	for (i = free_socket ; i != -1 ; ) {
		assert(i >= 0 && i < SEQ_SOCKET_MAX_SOCKETS);
		assert(count < SEQ_SOCKET_MAX_SOCKETS);
		assert(ss != i);

		++count;
		i = seqSockets[i].socket;
	}
}

#endif
