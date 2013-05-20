/*! \file si_fc.c
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
 * MPU9150 connected to smt32 olimex e407 through i2c connected through ethernet to FC
 * ADIS16405 connected to smt32 olimex e407 through spi connected through ethernet to FC
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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
#include "si_fc.h"

#define         COUNT_INTERVAL       10000
#define         MAX_USER_STRBUF      50
#define         MAX_RECV_BUFLEN      100
#define         MAX_THREADS          4
#define         NUM_THREADS          2
#define         TIMEBUFLEN           80
#define         STRINGBUFLEN         80

static          pthread_mutex_t      msg_mutex;
static          pthread_mutex_t      exit_request_mutex;

static          MPU9150_read_data    mpu9150_udp_data;

static          ADIS16405_burst_data adis16405_udp_data;

bool            user_exit_requested  = false;


/*! \brief Convert register value to degrees C
 *
 * @param raw_temp
 * @return
 */
static double mpu9150_temp_to_dC(int16_t raw_temp) {
	return(((double)raw_temp)/340 + 35);
}

static double CtoF(double c) {
   return (((c * 9.0)/5.0) + 32);
}
/*! \brief Convert an ADIS 12 bit temperature value to C
 *
 * @param decimal
 * @param temp reading
 * @return   TRUE if less than zero, FALSE if greater or equal to zero
 */
static bool adis16405_temp_to_dC(double* temperature, uint16_t* twos_num) {
	uint16_t ones_comp;
	bool     isnegative = false;
	uint32_t decimal;

	uint16_t local_twos;

	//! bit 11 is 12-bit two's complement sign bit
	isnegative   = (((uint16_t)(1<<11) & *twos_num) != 0) ? true : false;


	if(isnegative) {
		ones_comp     = ~(*twos_num & (uint16_t)0xfff) & 0xfff;
		decimal       = (ones_comp) + 1;
		*temperature  = -1 * (decimal * 0.14);
	} else {
		decimal      = *twos_num;
		*temperature  = decimal * 0.14;
	}
	*temperature      += 25.0;
	return isnegative;
}

struct timeval GetTimeStamp() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv;
}

static double timestamp_now() {
	struct timeval tv;
	double tstamp;

	tv          = GetTimeStamp();
	tstamp      = tv.tv_sec + (tv.tv_usec * .000001);

	return tstamp;
}

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

static void user_query_msg(volatile char *s) {
	pthread_mutex_lock(&msg_mutex);
	char   timebuf[TIMEBUFLEN];
	get_current_time(timebuf);
	fprintf(stderr, "M (%s):\t%s: ", timebuf, s);
	pthread_mutex_unlock(&msg_mutex);
}
static void log_msg(volatile char *s) {
	pthread_mutex_lock(&msg_mutex);
	char   timebuf[TIMEBUFLEN];
	get_current_time(timebuf);
	fprintf(stderr, "\nM (%s):\t%s\n", timebuf, s);
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
    retval   = strncmp( pa, portb, PORT_STRING_LEN-1);
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


void user_help() {
	pthread_mutex_lock(&msg_mutex);

    fprintf(stderr, "Help:\n\
            Please enter one of these choices:\n\
            g - start logging (go)\n\
            s - stop logging (stop)\n\
            r - reset sensors (reset)\n\
            q - quit program (quit)\n\
           \n");

    pthread_mutex_unlock(&msg_mutex);
}


/*! \brief User I/O
 *
 * @param ptr  pointer to Usertalk type
 */
void *user_io_thread (void* ptr) {
	Usertalk*                 user_info;

	int                       result;
	char                      str[MAX_USER_STRBUF];
	char                      keypress;

	sleep(3);
	fprintf(stderr, "\n");

	while(!user_exit_requested) {
		user_query_msg("(q)uit, (r)eset, (g)o, (s)top, (h)elp: ");
		fgets(str, sizeof(str), stdin);
		result = sscanf(str, "%c", &keypress );
		if(result < 1) {
			fprintf(stderr, "ERROR: Invalid entry\n");
			continue;
		}
		switch(keypress) {
			case 'q':
				log_msg("You typed q-quit\n");
				pthread_mutex_lock(&exit_request_mutex);
				user_exit_requested = true;
				pthread_mutex_unlock(&exit_request_mutex);
				break;
			case 'r':
				log_msg("You typed r-reset\n");
				break;
			case 'g':
				log_msg("You typed g-go\n");
				break;
			case 's':
				log_msg("You typed s-stop\n");
				break;
			case '\n':
				break;
			case 'h':
			case '?':
				user_help();
				break;
			default:
				fprintf(stderr, "ERROR: Unrecognized entry: '%c'\n",keypress);
				user_help();
				break;
		}
	}
}

/*! \brief Thread routine I/O
 *
 * @param ptr  pointer to Ports type with input and output ip address and port.
 */
void *datap_io_thread (void* ptr) {
	Ports*                    port_info;
	SensorID                  sensor_listen_id;
	port_info = (Ports*) ptr;

	int                       i = 0;
	int                       status;
	int                       retval;
	int                       hostsocket_fd;
	int                       clientsocket_fd;
	int                       numbytes;
	socklen_t                 addr_len;

	char                      timestring[TIMEBUFLEN];
	char                      ipstr[INET6_ADDRSTRLEN];
	char                      recvbuf[MAX_RECV_BUFLEN];
	char                      s[INET6_ADDRSTRLEN];

	pthread_t                 my_id;

	FILE                      *fp_mpu, *fp_adis;

	struct addrinfo           hints, *res, *p, *ai_client;
	struct sockaddr_storage   client_addr;
	socklen_t                 client_addr_len;
	client_addr_len           = sizeof(struct sockaddr_storage);

	memset(&hints, 0, sizeof hints);
	hints.ai_family                = AF_UNSPEC;  // AF_INET or AF_INET6 to force version
	hints.ai_socktype              = SOCK_DGRAM; // UDP
	hints.ai_flags                 = AI_PASSIVE; // use local host address.

	fprintf(stderr, "%s: listen port %s\n", __func__, port_info->host_listen_port);
	if(ports_equal(port_info->host_listen_port, FC_LISTEN_PORT_IMU_A_ADIS)) {
		sensor_listen_id = ADIS;
	} else if (ports_equal(port_info->host_listen_port, FC_LISTEN_PORT_IMU_A_MPU)) {
		sensor_listen_id = MPU;
	} else {
		sensor_listen_id = UNKNOWN_SENSOR;
	}

	if(sensor_listen_id == ADIS) {
		fp_adis       = fopen("adis16405_log.txt", "w");
		get_current_time(timestring) ;
		fprintf(fp_adis, "# adis16405 IMU data started at: %s\n", timestring);
		fprintf(fp_adis, "# adis16405 IMU raw data\n");
		fprintf(fp_adis, "# timestamp,ax,ay,az,gx,gy,gz,mx,my,mz,C\n");

	} else if (sensor_listen_id == MPU) {
		fp_mpu       = fopen("mpu9150_log.txt", "w");
		get_current_time(timestring) ;
		fprintf(fp_mpu, "# mpu9150 IMU data started at: %s\n", timestring);
		fprintf(fp_mpu, "# mpu9150 IMU raw data\n");
		fprintf(fp_mpu, "# timestamp,ax,ay,az,gx,gy,gz,C\n");
	} else {
		;
	}

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

	//for(i=0; i<NPACK; ++i) {
	uint32_t datacount       = 0;
	while(!user_exit_requested) {

		char   countmsg[MAX_USER_STRBUF];
		struct sockaddr        *sa;
		socklen_t              len;
		char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

		addr_len = sizeof client_addr;
		if ((numbytes = recvfrom(hostsocket_fd, recvbuf, MAX_RECV_BUFLEN-1 , 0,
				(struct sockaddr *)&client_addr, &addr_len)) == -1) {
			die_nice("recvfrom");
		}


		if (getnameinfo((struct sockaddr *)&client_addr, client_addr_len, hbuf, sizeof(hbuf), sbuf,
				sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV) == 0) {

		//	printf("%d\t%s:%s\t", i, hbuf, sbuf);

			if(ports_equal(sbuf, IMU_A_TX_PORT_MPU) && (sensor_listen_id == MPU)) {
			//	printf("MPU Packet %s:%s\n", hbuf, sbuf);
				memcpy (&mpu9150_udp_data, recvbuf, sizeof (MPU9150_read_data));

				//		printf("fc: size of data struct: %d\n", sizeof(MPU9150_read_data));
				//		printf("fc: packet is %d bytes long\n", numbytes);

				fprintf(fp_mpu, "%f,%d,%d,%d,%d,%d,%d,%3.2f\n",
						timestamp_now(),
						mpu9150_udp_data.accel_xyz.x,
						mpu9150_udp_data.accel_xyz.y,
						mpu9150_udp_data.accel_xyz.z,
						mpu9150_udp_data.gyro_xyz.x,
						mpu9150_udp_data.gyro_xyz.y,
						mpu9150_udp_data.gyro_xyz.z,
						CtoF(mpu9150_temp_to_dC(mpu9150_udp_data.celsius)) );
				fflush(fp_mpu);
				++datacount;
				snprintf(countmsg, MAX_USER_STRBUF , " %d MPU entries.", datacount );
				if(datacount %COUNT_INTERVAL == 0) {
					log_msg(countmsg);
				}
#if DEBUG_MPU_NET
				printf("\r\nraw_temp: %3.2f C\r\n", mpu9150_temp_to_dC(mpu9150_udp_data.celsius));
				printf("ACCL:  x: %d\ty: %d\tz: %d\r\n", mpu9150_udp_data.accel_xyz.x, mpu9150_udp_data.accel_xyz.y, mpu9150_udp_data.accel_xyz.z);
				printf("GRYO:  x: 0x%x\ty: 0x%x\tz: 0x%x\r\n", mpu9150_udp_data.gyro_xyz.x, mpu9150_udp_data.gyro_xyz.y, mpu9150_udp_data.gyro_xyz.z);
				if ((numbytes = sendto(clientsocket_fd, recvbuf, strlen(recvbuf), 0,
						ai_client->ai_addr, ai_client->ai_addrlen)) == -1) {
					die_nice("client sendto");
				}
#endif
			} else if (ports_equal(sbuf, IMU_A_TX_PORT_ADIS) && (sensor_listen_id == ADIS)) {
				double adis_temp_C = 0.0;
				bool   adis_temp_neg = false;

			//	printf("ADIS Packet: %s:%s\n", hbuf, sbuf);
				memcpy (&adis16405_udp_data, recvbuf, sizeof (ADIS16405_burst_data));

				//		printf("fc: size of data struct: %d\n", sizeof(MPU9150_read_data));
				//		printf("fc: packet is %d bytes long\n", numbytes);


				adis_temp_neg = adis16405_temp_to_dC(&adis_temp_C,      &adis16405_udp_data.adis_temp_out);
				 fprintf(fp_adis, "%f,%d,%d,%d,%d,%d,%d,%d,%d,%d,%3.2f\n",
				//  fprintf(fp_adis, "%f,%d,%d,%d,%d,%d,%d,%0x%x\n",

						timestamp_now(),
						adis16405_udp_data.adis_xaccl_out,
						adis16405_udp_data.adis_yaccl_out,
						adis16405_udp_data.adis_zaccl_out,
						adis16405_udp_data.adis_xgyro_out,
						adis16405_udp_data.adis_ygyro_out,
						adis16405_udp_data.adis_zgyro_out,
						adis16405_udp_data.adis_xmagn_out,
						adis16405_udp_data.adis_ymagn_out,
						adis16405_udp_data.adis_zmagn_out,
						CtoF(adis_temp_C)
						//adis16405_udp_data.adis_temp_out
				 );
				 fflush(fp_adis);
				 ++datacount;
				 snprintf(countmsg, MAX_USER_STRBUF , " %d ADIS entries.", datacount );
				 if(datacount %COUNT_INTERVAL == 0) {
					 log_msg(countmsg);
				 }
			} else {
				printf("Unrecognized Packet %s:%s\n", hbuf, sbuf);
			}
		}
	}
	get_current_time(timestring) ;
	if(sensor_listen_id == MPU ) {
		fprintf(fp_mpu, "# mpu9150 IMU data closed at: %s\n", timestring);
		fclose(fp_mpu);
	} else if (sensor_listen_id == ADIS) {
		fprintf(fp_adis, "# adis16405 IMU data closed at: %s\n", timestring);
		fclose(fp_adis);
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
	Usertalk	     th_talk;

	pthread_mutex_init(&msg_mutex, NULL);
	pthread_mutex_init(&exit_request_mutex, NULL);

	tc = pthread_create( &thread_id[i], NULL, &user_io_thread, &th_talk );
	if (tc){
		printf("== Error=> pthread_create() fail with code: %d\n", tc);
		exit(EXIT_FAILURE);
	}

	th_talk.thread_id = 42;

	for(i=0; i<MAX_THREADS; ++i) {
		init_thread_state(&th_data[i], i);
	}

	snprintf(msgbuf, STRINGBUFLEN, "Number of processors: %d", get_numprocs());
	log_msg(msgbuf);

	snprintf(th_data[ADIS_LISTENER].host_listen_port, PORT_STRING_LEN , "%d", FC_LISTEN_PORT_IMU_A_ADIS);
	snprintf(th_data[ADIS_LISTENER].client_addr     , INET6_ADDRSTRLEN, "%s", ROLL_CTL_IP_ADDR_STRING);
	snprintf(th_data[ADIS_LISTENER].client_port     , PORT_STRING_LEN , "%d", ROLL_CTL_LISTEN_PORT);

	snprintf(th_data[MPU_LISTENER].host_listen_port, PORT_STRING_LEN , "%d", FC_LISTEN_PORT_IMU_A_MPU);
	snprintf(th_data[MPU_LISTENER].client_addr     , INET6_ADDRSTRLEN, "%s", ROLL_CTL_IP_ADDR_STRING);
	snprintf(th_data[MPU_LISTENER].client_port     , PORT_STRING_LEN , "%d", ROLL_CTL_LISTEN_PORT);


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
	pthread_mutex_destroy(&exit_request_mutex);

	pthread_exit(NULL);


	return 0;
}

