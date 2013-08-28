/*! \file datap_fc.c
 *
 * Additional background on network programming can be found here:
 *
 * Reference:
 * Hall, Brian B. "Beej's Guide to Network Programming." Beej's Guide to Network Programming. Brain Hall, 3 July 2012. Web. 30 Mar. 2013.
 * http://beej.us/guide/bgnet/
 *
 * This code uses the 'getaddrinfo' and IPv4/IPv6 techniques presented in the above document.
 */

/*!
 * Flight computer (Linux-os) running datapath test
 * between two STM based boards
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <unistd.h>

#include "fc_net.h"
#include "device_net.h"
#include "datap_fc.h"

#define         MAX_THREADS          4
#define         NUM_THREADS          2
#define         TIMEBUFLEN           80
#define         STRINGBUFLEN         80

static          pthread_mutex_t      msg_mutex;

/*!
 * \warning ts had better be TIMEBUFLEN in length
 *
 * This function won't check.
 *
 * @param ts
 */
static void get_current_time(char* ts) {
	time_t      rawtime;
	struct tm*  timeinfo;

	time(&rawtime);

	timeinfo = localtime ( &rawtime );
	strftime (ts, TIMEBUFLEN, "%T %F",timeinfo);
}

static void log_msg(volatile char *s) {
	pthread_mutex_lock(&msg_mutex);
	char   timebuf[TIMEBUFLEN];
	get_current_time(timebuf);
	fprintf(stderr, "M (%s):\t%s\n", timebuf, s);
	pthread_mutex_unlock(&msg_mutex);
}

static void log_error(volatile char *s) {
	pthread_mutex_lock(&msg_mutex);
	char   timebuf[TIMEBUFLEN];
	get_current_time(timebuf);
	fprintf(stderr, "E (%s):\t%s\n", timebuf, s);
	pthread_mutex_unlock(&msg_mutex);

}

static void die_nice(char *s) {
	log_error("exiting\n");
	perror(s);
	exit(1);
}

static bool ports_equal(char* pa, int pb) {
    bool     retval = false;
	char     portb[PORT_STRING_LEN];

    snprintf(portb, PORT_STRING_LEN, "%d", pb);
    retval   = strncmp( pa, portb, PORT_STRING_LEN);

    if(retval != 0) {
        return false;
    }
    return true;
}

/*! \brief Get the number of processors available on this (Linux-os) machine
 *
 */
static int get_numprocs() {
	FILE*    fp;
	int      numprocs;
	int      scanfret;

	fp       = popen ("grep -c \"^processor\" /proc/cpuinfo","r");
	scanfret = fscanf(fp, "%i", &numprocs);
	fclose(fp);
	return numprocs;
}

/*! \brief return the ip address from a sockaddr
 *
 * Return IPv4 or IPv6 as appropriate
 *
 * @param sa
 */
static void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

static void init_thread_state(Ports* p, unsigned int i) {
	p->thread_id = i;
}

/*! \brief Thread routine I/O
 *
 * @param ptr  pointer to Ports type with input and output ip address and port.
 */
void *datap_io_thread (void* ptr) {
	Ports*                    port_info;
	port_info = (Ports*) ptr;

	int                       i = 0;
	int                       status;
	int                       retval;
	int                       hostsocket_fd;
	int                       clientsocket_fd;
	int                       numbytes;
	socklen_t                 addr_len;

	char                      ipstr[INET6_ADDRSTRLEN];
	char                      recvbuf[MAXBUFLEN];
	char                      s[INET6_ADDRSTRLEN];

	pthread_t                 my_id;

	struct addrinfo           hints, *res, *p, *ai_client;
	struct sockaddr_storage   client_addr;
	socklen_t                 client_addr_len;
	client_addr_len           = sizeof(struct sockaddr_storage);

	memset(&hints, 0, sizeof hints);
	hints.ai_family                = AF_UNSPEC;  // AF_INET or AF_INET6 to force version
	hints.ai_socktype              = SOCK_DGRAM; // UDP
	hints.ai_flags                 = AI_PASSIVE; // use local host address.

	fprintf(stderr, "%s: listen port %s\n", __func__, port_info->host_listen_port);

	/*!
	 * Getting address of THIS machine.
	 */
	if ((status = getaddrinfo(NULL, port_info->host_listen_port, &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return ;
	}

	/*!
	 * Get address of any machine from DNS
	 */
	//	if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) {
	//		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
	//		return 2;
	//	}


	for(p = res; p != NULL; p = p->ai_next) {
		void *addr;
		char *ipver;

		// get the pointer to the address itself,
		// different fields in IPv4 and IPv6:
		if (p->ai_family == AF_INET) { // IPv4
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
			addr = &(ipv4->sin_addr);
			ipver = "IPv4";
		} else { // IPv6
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
			addr = &(ipv6->sin6_addr);
			ipver = "IPv6";
		}

		// convert the IP to a string and print it:
		inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
		printf("  %s: %s\n", ipver, ipstr);
	}

	if((hostsocket_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
		die_nice("socket");
	}

	if(bind(hostsocket_fd, res->ai_addr, res->ai_addrlen) == -1) {
		die_nice("bind");
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family   = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((retval = getaddrinfo(port_info->client_addr, port_info->client_port, &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(retval));
		die_nice("client get address");
	}

	/* Create a socket for the client */
	for(ai_client = res; ai_client != NULL; ai_client = ai_client->ai_next) {
		if ((clientsocket_fd = socket(ai_client->ai_family, ai_client->ai_socktype,
				ai_client->ai_protocol)) == -1) {
			perror("clientsocket");
			continue;
		}
		break;
	}

	if (ai_client == NULL) {
		die_nice("failed to bind control socket\n");
	}

	for(i=0; i<NPACK; ++i) {
		struct sockaddr        *sa;
		socklen_t              len;
		char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

		addr_len = sizeof client_addr;
		if ((numbytes = recvfrom(hostsocket_fd, recvbuf, MAXBUFLEN-1 , 0,
				(struct sockaddr *)&client_addr, &addr_len)) == -1) {
			die_nice("recvfrom");
		}

		if (getnameinfo((struct sockaddr *)&client_addr, client_addr_len, hbuf, sizeof(hbuf), sbuf,
		            sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV) == 0) {
			if(ports_equal(sbuf, IMU_A_TX_PORT)) {
				printf("IMU Packet %s:%s\n", hbuf, sbuf);
			} else if (ports_equal(sbuf, ROLL_CTL_TX_PORT)) {
				printf("\tROLL CTL Packet: %s:%s\n", hbuf, sbuf);
			} else {
				printf("Unrecognized Packet %s:%s\n", hbuf, sbuf);
			}
		}

		printf("fc: packet is %d bytes long\n", numbytes);
		recvbuf[numbytes] = '\0';
		printf("fc: packet contains \"%s\"\n\n", recvbuf);

		if ((numbytes = sendto(clientsocket_fd, recvbuf, strlen(recvbuf), 0,
				ai_client->ai_addr, ai_client->ai_addrlen)) == -1) {
			die_nice("client sendto");
		}
	}
	close(hostsocket_fd);
	close(clientsocket_fd);
	freeaddrinfo(res); // free the linked list
	fprintf(stderr, "Leaving thread %d\n", port_info->thread_id);
	return 0;
}

int main(void) {
	unsigned int     i          = 0;
	unsigned int     j          = 0;

	char             msgbuf[STRINGBUFLEN];

	int              numthreads = NUM_THREADS;
	int              tc         = 0;
	int              tj         = 0;

	pthread_t        thread_id[MAX_THREADS];
	Ports            th_data[MAX_THREADS];

	pthread_mutex_init(&msg_mutex, NULL);

	for(i=0; i<MAX_THREADS; ++i) {
		init_thread_state(&th_data[i], i);
	}

	snprintf(msgbuf, STRINGBUFLEN, "Number of processors: %d", get_numprocs());
	log_msg(msgbuf);

	snprintf(th_data[CONTROL_LISTENER].host_listen_port, PORT_STRING_LEN , "%d", FC_LISTEN_PORT_ROLL_CTL);
	snprintf(th_data[CONTROL_LISTENER].client_addr     , INET6_ADDRSTRLEN, "%s", IMU_A_IP_ADDR_STRING);
	snprintf(th_data[CONTROL_LISTENER].client_port     , PORT_STRING_LEN , "%d", IMU_A_LISTEN_PORT);

	snprintf(th_data[SENSOR_LISTENER].host_listen_port, PORT_STRING_LEN , "%d", FC_LISTEN_PORT_IMU_A);
	snprintf(th_data[SENSOR_LISTENER].client_addr     , INET6_ADDRSTRLEN, "%s", IMU_A_IP_ADDR_STRING);
	snprintf(th_data[SENSOR_LISTENER].client_port     , PORT_STRING_LEN , "%d", IMU_A_LISTEN_PORT);

	printf("start threads\n");

	for(i=0; i<numthreads; ++i) {
		tc = pthread_create( &thread_id[i], NULL, &datap_io_thread, (void*) &th_data[i] );
		if (tc){
			printf("== Error=> pthread_create() fail with code: %d\n", tc);
			exit(EXIT_FAILURE);
		}
	}

	// Things happen...then join threads as they return.

	for(j=0; j<numthreads; ++j) {
		tj = pthread_join(thread_id[j], NULL);
		if (tj){
			printf("== Error=> pthread_join() fail with code: %d\n", tj);
			exit(EXIT_FAILURE);
		}
	}
	pthread_mutex_destroy(&msg_mutex);
	pthread_exit(NULL);


	return 0;
}

