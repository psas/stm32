/*! \file datap_fc.c
 */

/*!
 * Flight computer (Linux-os) running datapath test
 * between two STM based boards
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "datap_fc.h"

#define         MAX_THREADS          4
#define         NUM_THREADS          1
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

static void log_msg(char *s) {
	pthread_mutex_lock(&msg_mutex);
	char   timebuf[TIMEBUFLEN];
	get_current_time(timebuf);
	fprintf(stderr, "M (%s):\t%s\n", timebuf, s);
	pthread_mutex_unlock(&msg_mutex);
}

static void log_error(char *s) {
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

static void init_thread_state(Ports* p, unsigned int i) {
	p->thread_id = i;
}

/*! \brief Thread routine I/O
 *
 * @param ptr  pointer to Ports type with input and output ip address and port.
 */
void *datap_io_thread (void* ptr) {
	Ports*      port_info;
	port_info = (Ports*) ptr;

	pthread_t   my_id;

	int         s, i;
	int         me_len    = sizeof(port_info->si_me);
	int         other_len = sizeof(port_info->si_other);

	char        msgbuf[STRINGBUFLEN];
	char        netbuf[NETBUFLEN];

	snprintf(msgbuf, STRINGBUFLEN, "%s: thread %d",  __func__, port_info->thread_id);
	log_msg(msgbuf);

	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1) {
		die_nice("socket");
	}

	if (bind(s, (struct sockaddr *)&port_info->si_me, (socklen_t)  sizeof(port_info->si_me))==-1) {
		die_nice("bind");
	}

	for (i=0; i<NPACK; ++i) {
		if (recvfrom(s, netbuf, NETBUFLEN, 0, (struct sockaddr *)&port_info->si_other, &other_len)==-1) {
			die_nice("recvfrom()");
		}
		printf("Received packet from %s:%d\nData: %s\n\n",
				inet_ntoa(port_info->si_other.sin_addr), ntohs(port_info->si_other.sin_port), netbuf);
	}

	close(s);

}


int main(void) {
	unsigned int     i          = 0;
	unsigned int     j          = 0;

	char             buf[STRINGBUFLEN];

	int              numthreads = NUM_THREADS;
	int              tc         = 0;
	int              tj         = 0;

	pthread_t        thread_id[MAX_THREADS];
	Ports            th_data[MAX_THREADS];

	pthread_mutex_init(&msg_mutex, NULL);

	for(i=0; i<MAX_THREADS; ++i) {
		init_thread_state(&th_data[i], i);
	}

	snprintf(buf, STRINGBUFLEN, "Number of processors: %d", get_numprocs());
	log_msg(buf);

	memset((char *) &th_data[SENSOR_BOARD].si_me,    0, sizeof(th_data[SENSOR_BOARD].si_me));
	memset((char *) &th_data[SENSOR_BOARD].si_other, 0, sizeof(th_data[SENSOR_BOARD].si_other));

	th_data[SENSOR_BOARD].si_me.sin_family       = AF_INET;
	th_data[SENSOR_BOARD].si_me.sin_port         = htons(PORT_OUT);

	th_data[SENSOR_BOARD].si_other.sin_family    = AF_INET;
	th_data[SENSOR_BOARD].si_other.sin_port      = htons(PORT_IN);
	if (inet_aton(SENSOR_IP, &th_data[SENSOR_BOARD].si_other.sin_addr)==0) {
		die_nice("inet_aton() failed\n");
	}

	printf("start threads\n");

	for(i=0; i<numthreads; ++i) {
		tc = pthread_create( &thread_id[i], NULL, &datap_io_thread, (void*) &th_data[i] );
		if (tc){
			printf("== Error=> pthread_create() fail with code: %d\n", tc);
			exit(EXIT_FAILURE);
		}
	}


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

