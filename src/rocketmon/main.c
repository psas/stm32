#include <stdio.h>          
#include <stdlib.h>         
#include <string.h>         
#include <fcntl.h>         
#include <sys/stat.h>     
#include <errno.h>
#include <sys/types.h>      
#include <netinet/in.h>     
#include <sys/socket.h>      
#include <arpa/inet.h>       
#include <sched.h>   
#include <pthread.h>        
#include <signal.h>          
#include <semaphore.h> 
#include <errno.h>
#include "main.h"
#include "shell.h"
unsigned int	client_s;
int main() {
	//when we fold this in to the main system, this probably should be a thread
	serverInit();
	return 0;
}

int serverInit() {
	unsigned int			server_s;				// Server socket descriptor
	struct sockaddr_in	server_addr;			// Server Internet address
	//unsigned int			client_s;			// Client socket descriptor
	struct sockaddr_in	client_addr;			// Client Internet address
	struct in_addr		client_ip_addr;			// Client IP address
	int					addr_len;				// Internet address length
	unsigned int			ids;					// holds thread args
	pthread_attr_t		attr;					//	pthread attributes
	pthread_t				threads;				// Thread ID (used by OS)
	server_s = socket(AF_INET, SOCK_STREAM, 0);
	//set up socket
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	printf("Binding\n");
	int success = bind(server_s, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (success == -1) {
		printf("ERROR: binding failed\n");
		printf("ERRNO: %s\n", strerror(errno));
		exit(FALSE);
	}
	printf("Listening\n");
	listen(server_s, PEND_CONNECTIONS);
	printf("Setting thread attributes\n");
	pthread_attr_init(&attr);
	while (TRUE) {
		addr_len = sizeof(client_addr);
    	client_s = accept(server_s, (struct sockaddr *)&client_addr, &addr_len);
		printf("Client_s in main thread: %d\n", client_s);
    	if (client_s == -1) {
      	printf("ERROR - Unable to create socket \n");
			printf("ERRNO: %s\n", strerror(errno));
      	exit(FALSE);
    	} else {
			ids = client_s;
			printf("Starting client thread\n");
        	pthread_create (					/* Create a child thread        */ 
         	&threads,				/* Thread ID (system assigned)  */     
         	&attr,					/* Default thread attributes    */
         	shellMain,				/* Thread routine               */
         	&ids
			);					/* Arguments to be passed       */

    	}
	}
	close (server_s); 
}
