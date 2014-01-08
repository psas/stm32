
#include <arpa/inet.h>

static int get_udp_socket(const char * ip, unsigned int port){

    //Create our own address
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET,
    addr.sin_port = htons(port);
    inet_aton(ip, &addr.sin_addr);

    //Create the socket
    int s = socket(AF_INET,  SOCK_DGRAM, 0);
    if(s < 0){
        chprintf(chp, "Socket allocation failure \r\n");
        return -1;
    }

    //bind our own address to the socket
    if(bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        chprintf(chp, "Socket bind failure\r\t");
        return -2;
    }
    return s;
}


int main(void){

    int s = get_udp_socket();

    char msg[1000];
    while(1) {
        //we can use recv here because we dont care where the packets came from
        //If we did, recvfrom() is the function to use
        if(recv(s, msg, sizeof(msg), 0) < 0){
            chprintf(chp, "Receive socket recv failure \r\n");
            return -3;
        } else {
            chprintf(chp, "%s\r\n", msg);
        }
    }
}
