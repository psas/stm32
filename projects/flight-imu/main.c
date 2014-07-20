#include <stdint.h>

#include "ch.h"
#include "hal.h"

#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "lwipthread.h"

#include "rci.h"
#include "utils_rci.h"
#include "utils_sockets.h"
#include "utils_general.h"
#include "utils_led.h"
#include "net_addrs.h"
#ifdef FLIGHT
#include "iwdg.h"
#endif

#include "ADIS16405.h"
#include "MPL3115A2.h"

static struct SeqSocket adis_socket = DECL_SEQ_SOCKET(sizeof(ADIS16405Data));
static struct SeqSocket mpl_socket = DECL_SEQ_SOCKET(sizeof(struct MPL3115A2Data));

static const struct swap adis_swaps[] = {
    SWAP_FIELD(ADIS16405Data, supply_out),
    SWAP_FIELD(ADIS16405Data, xgyro_out),
    SWAP_FIELD(ADIS16405Data, ygyro_out),
    SWAP_FIELD(ADIS16405Data, zgyro_out),
    SWAP_FIELD(ADIS16405Data, xaccl_out),
    SWAP_FIELD(ADIS16405Data, yaccl_out),
    SWAP_FIELD(ADIS16405Data, zaccl_out),
    SWAP_FIELD(ADIS16405Data, xmagn_out),
    SWAP_FIELD(ADIS16405Data, ymagn_out),
    SWAP_FIELD(ADIS16405Data, zmagn_out),
    SWAP_FIELD(ADIS16405Data, temp_out),
    SWAP_FIELD(ADIS16405Data, aux_adc),
    {0},
};

static const struct swap mpl_swaps[] = {
    SWAP_FIELD(struct MPL3115A2Data, pressure),
    SWAP_FIELD(struct MPL3115A2Data, temperature),
    {0},
};

static void adis_drdy_handler(eventid_t id UNUSED){
    ADIS16405Data data;
    adis_get_data(&data);

    write_swapped(adis_swaps, &data, adis_socket.buffer);
    seq_write(&adis_socket, len_swapped(adis_swaps));
}

static void mpl_drdy_handler(eventid_t id UNUSED){
    struct MPL3115A2Data data;
    MPL3115A2GetData(&data);

    write_swapped(mpl_swaps, &data, mpl_socket.buffer);
    seq_write(&mpl_socket, len_swapped(mpl_swaps));
}

void main(void){
    halInit();
    chSysInit();
#ifdef FLIGHT
    iwdgStart();
#endif
    ledStart(NULL);

    struct RCICommand commands[] = {
            RCI_CMD_VERS,
            {NULL}
    };

    /* Start lwip */
    lwipThreadStart(SENSOR_LWIP);
    RCICreate(commands);

    /* Create the ADIS out socket, connecting as it only sends to one place */
    int s = get_udp_socket(ADIS_ADDR);
    chDbgAssert(s >= 0, "ADIS socket failed", NULL);
    seq_socket_init(&adis_socket, s);

    s = get_udp_socket(MPL_ADDR);
    chDbgAssert(s >= 0, "MPL socket failed", NULL);
    seq_socket_init(&mpl_socket, s);

    connect(adis_socket.socket, FC_ADDR, sizeof(struct sockaddr));
    connect(mpl_socket.socket, FC_ADDR, sizeof(struct sockaddr));

    adis_init(&adis_olimex_e407);
    static struct MPL3115A2Config conf = {
        .i2cd = &I2CD2,
        .pins = {.SDA = {GPIOF, GPIOF_PIN0}, .SCL = {GPIOF, GPIOF_PIN1}},
        .interrupt = {GPIOF, GPIOF_PIN3}
    };
    MPL3115A2Start(&conf);

    /* Manage data events */
    struct EventListener adisdrdy, mpldrdy;
    static const evhandler_t evhndl[] = {
            adis_drdy_handler,
            mpl_drdy_handler
    };
    chEvtRegister(&ADIS16405_data_ready, &adisdrdy, 0);
    chEvtRegister(&MPL3115A2DataEvt, &mpldrdy, 1);
    while(TRUE){
        chEvtDispatch(evhndl, chEvtWaitAny(ALL_EVENTS));
    }
}
