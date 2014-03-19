#include <string.h>

#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "lwipthread.h"

#include "utils_sockets.h"

void set_lwipthread_opts(struct lwipthread_opts * ip_opts,
    const char * ip, const char * netmask, const char * gateway, uint8_t * mac)
{
    ip_opts->macaddress = mac;
    ip_opts->address = inet_addr(ip);
    ip_opts->netmask = inet_addr(netmask);
    ip_opts->gateway = inet_addr(gateway);
}

void set_sockaddr(struct sockaddr_in * addr, const char * ip, int port){
    //Create an address (remember to have the data in network byte order)
    memset(addr, 0, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET,
    addr->sin_port = htons(port);
    inet_aton(ip, &addr->sin_addr);
}


int get_udp_socket(const struct sockaddr *addr){
    //Create the socket
    int s = socket(AF_INET,  SOCK_DGRAM, 0);
    if(s < 0){
        /* socket allocation failure */
        return -1;
    }

    //bind our own address to the socket
    if(bind(s, addr, sizeof(struct sockaddr)) < 0){
        /* socket bind failure */
        return -2;
    }
    return s;
}
