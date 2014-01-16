#include <string.h>

#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "lwipthread.h"

#include "device_net.h"
#include "fc_net.h"

struct sockaddr_in adis_out;
struct sockaddr_in adis_in;
struct lwipthread_opts ip_opts;

struct lwipthread_opts * get_adis_addr(void){
    struct ip_addr ip, gateway, netmask;
    IP4_ADDR(&ip,      10, 0, 0,   20);
    IP4_ADDR(&gateway, 10, 0, 0,   1  );
    IP4_ADDR(&netmask, 255, 255, 255, 0  );
    uint8_t macAddress[6] = {0xC2, 0xAF, 0x51, 0x03, 0xCF, 0x46};

    ip_opts.macaddress = macAddress;
    ip_opts.address    = ip.addr;
    ip_opts.netmask    = netmask.addr;

    return &ip_opts;
}

struct sockaddr * make_sockaddr(struct sockaddr_in * addr, char * ip, int port){
    //Create an address (remember to have the data in network byte order)
    memset(addr, 0, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET,
    addr->sin_port = htons(port);
    inet_aton(ip, &addr->sin_addr);
    return (struct sockaddr*) addr;
}


int get_udp_socket(struct sockaddr *addr){
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
