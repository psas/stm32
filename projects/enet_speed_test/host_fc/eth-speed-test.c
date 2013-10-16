/*
 * eth-speed-test.c
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <signal.h>  /* POSIX signals */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int open_tty(const char * tty_name){
	int tty;
	struct termios attrib;

	tty = open(tty_name, O_RDWR | O_NOCTTY);
	if (tty == -1){
		perror("open_port: Unable to open the tty device"); //todo: print tty name
		raise(SIGINT);
	}
	tcgetattr(tty, &attrib);
	cfsetispeed(&attrib, B115200);
	tcsetattr(tty, TCSANOW, &attrib); //todo: valgrind errors here
	return tty;
}


/*
 * A simple server in the internet domain using TCP
 * The port number is passed as an argument
 * From http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
 *
 */


void error(char *msg)
{
	printf("error\n");
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
	int tty = open_tty("/dev/ttyS1");
    int sockfd, portno, n;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    signal(SIGPIPE, SIG_IGN);

    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    memset(buffer, '\0', 256);
    buffer[0] = 'a';

    while(1){
    	write(tty, "U", 1);
    	n = write(sockfd, buffer, strlen(buffer));

        if (n < 0){
        	printf("re-connecting\n");
        	close(sockfd);
        	sockfd = socket(AF_INET, SOCK_STREAM, 0);
        	write(tty, "UU", 1);
            if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
                error("ERROR connecting");
//             error("ERROR writing to socket");
        }
    	sleep(1);
    }
    printf("Exit");
//

//    bzero(buffer,256);
//    n = read(sockfd,buffer,255);
//    if (n < 0)
//         error("ERROR reading from socket");
//    printf("%s\n",buffer);
    return 0;
}

//int main(int argc, char *argv[])
//{
//
//
//	if (argc < 3) {
//	   fprintf(stderr,"usage %s hostname port\n", argv[0]);
//	   exit(0);
//	}
//
//	if(strcmp(argv[1], "server")){
//		client(argv[1], argv[2]);
//	}else{
//		server(argv[2]);
//	}
//
//
//}
//
//int server(char * port){
//	int ttyS0 = open_tty("/dev/ttyS0");
//    int sockfd, newsockfd, portno, clilen;
//    char buffer[256];
//    struct sockaddr_in serv_addr, cli_addr;
//    int n;
//
//    sockfd = socket(AF_INET, SOCK_STREAM, 0);
//    if (sockfd < 0)
//       error("ERROR opening socket");
//
//    bzero((char *) &serv_addr, sizeof(serv_addr));
//    portno = atoi(port);
//    serv_addr.sin_family = AF_INET;
//    serv_addr.sin_addr.s_addr = INADDR_ANY;
//    serv_addr.sin_port = htons(portno);
//    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
//        error("ERROR on binding");
//
//    listen(sockfd,5);
//    clilen = sizeof(cli_addr);
//    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
//    if(newsockfd < 0) error("ERROR on accept");
//
//    bzero(buffer,256);
//    n = read(newsockfd,buffer,255);
//    if(n < 0) error("ERROR reading from socket");
//
//    write(ttyS0, "U", 1);
//    printf("Here is the message: %s\n",buffer);
//
//    n = write(newsockfd,"I got your message",18);
//    if(n < 0) error("ERROR writing to socket");
//    return 0;
//}
//
//
//int client(const char * servername, char * port){
//	int ttyS0 = open_tty("/dev/ttyS0");
//	int sockfd, portno, n;
//
//	struct sockaddr_in serv_addr;
//	struct hostent *server;
//
//	char buffer[256];
//
//	portno = atoi(port);
//	sockfd = socket(AF_INET, SOCK_STREAM, 0);
//	if (sockfd < 0)
//		error("ERROR opening socket");
//	server = gethostbyname(servername);
//	if (server == NULL) {
//		fprintf(stderr,"ERROR, no such host\n");
//		exit(0);
//	}
//	bzero((char *) &serv_addr, sizeof(serv_addr));
//	serv_addr.sin_family = AF_INET;
//	bcopy((char *)server->h_addr,
//		 (char *)&serv_addr.sin_addr.s_addr,
//		 server->h_length);
//	serv_addr.sin_port = htons(portno);
//	if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
//		error("ERROR connecting");
//	printf("Please enter the message: ");
//	bzero(buffer,256);
//	fgets(buffer,255,stdin);
//
//	n = write(sockfd,buffer,strlen(buffer));
//	write(ttyS0, "U", 1);
//	if (n < 0)
//		 error("ERROR writing to socket");
//	bzero(buffer,256);
//	n = read(sockfd,buffer,255);
//	if (n < 0)
//		 error("ERROR reading from socket");
//	printf("%s\n",buffer);
//	return 0;
//}
