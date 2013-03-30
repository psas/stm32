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
#include "main.h"
#include "shell.h"

extern unsigned int	client_s;
void *shellMain(void * arg) {
	unsigned int	myClient_s;			//copy socket
	char           in_buf[BUF_SIZE];           // Input buffer for GET resquest
	char           out_buf[BUF_SIZE];          // Output buffer for HTML response
	char				command[BUF_SIZE];
   char           cmdout[BUF_SIZE];
	char           *file_name;                 // File name
	unsigned int   fh;                         // File handle (file descriptor)
	unsigned int   buf_len;                    // Buffer length for file reads
	myClient_s = *(unsigned int *)arg;	
	int retcode;
	printf("Client id in thread %d\n", client_s);
	while (TRUE) {
		strcpy(out_buf,COMMANDPROMPT);
		printf("Sending command prompt\n");
		send(client_s, out_buf, strlen(out_buf), 0);
		printf("Listening\n");
		retcode = recv(client_s, in_buf, BUF_SIZE, 0);
		if (retcode < 0) {
			printf("Something terrible has happened!\n");
		} else {
	      //strncpy(command,in_buf,strlen(in_buf));
         char* p = strchr(in_buf,'\n');
         *p = '\0';
	      if (execCmd(in_buf,&cmdout[0]) == -1) {
            strcpy(out_buf, CMDUNKNOWN);	
            send(client_s, out_buf, strlen(out_buf), 0);
         } else {
            strcpy(out_buf,cmdout);
            send(client_s,out_buf,strlen(out_buf), 0);
         }
		}
	}
}

int execCmd(char command[],char * cmdout) {
   //wrapper for commands
   char databuf[1024];
   if (strncmp(command,HELP,4) == 0) {
      sprintf(cmdout,"Commands:\nshow threadid\nshow sensors\nshow modules\ntest motors\n");
      return 0;
   } else if (strncmp(command,SHOWTHREADID,13) == 0) {
      sprintf(cmdout,"Thread ID: %d\n", pthread_self());
      return 0; 
   } else {
      printf("got unknown command %s\n", command);
	   return -1;
   }   
}


