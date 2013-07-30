/*! \file sdcdetail.c
 *
 */

/*!
 * \defgroup sdcdetail SDC Utilities
 * @{
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "ff.h"

#define POLLING_INTERVAL                10
#define POLLING_DELAY                   10

/*!
 * \brief Card events
 */
EventSource inserted_event, removed_event;

/*!
 *  FS mounted and ready.
 */
bool fs_ready = FALSE;

/*!
 * \brief FS object.
 */
FATFS SDC_FS;


/*1
 * \brief   Card monitor timer.
 */
static VirtualTimer sdc_tmr;

/**
 * @brief   Debounce counter.
 */
static unsigned cnt;

/*!
 * @brief   Insertion monitor timer callback function.
 *
 * @param[in] p         pointer to the @p BaseBlockDevice object
 *
 * @notapi
 */
static void sdc_tmrfunc(void *p) {
    BaseBlockDevice *bbdp = p;

    chSysLockFromIsr();
    if (cnt > 0) {
        if (blkIsInserted(bbdp)) {
            if (--cnt == 0) {
                chEvtBroadcastI(&inserted_event);
            }
        }
        else
            cnt = POLLING_INTERVAL;
    }
    else {
        if (!blkIsInserted(bbdp)) {
            cnt = POLLING_INTERVAL;
            chEvtBroadcastI(&removed_event);
        }
    }
    chVTSetI(&sdc_tmr, MS2ST(POLLING_DELAY), sdc_tmrfunc, bbdp);
    chSysUnlockFromIsr();
}

/**
 * @brief   Polling monitor start.
 *
 * @param[in] p         pointer to an object implementing @p BaseBlockDevice
 *
 * @notapi
 */
void sdc_tmr_init(void *p) {

    chEvtInit(&inserted_event);
    chEvtInit(&removed_event);
    chSysLock();
    cnt = POLLING_INTERVAL;
    chVTSetI(&sdc_tmr, MS2ST(POLLING_DELAY), sdc_tmrfunc, p);
    chSysUnlock();
}

/*!
 * \brief Card insertion event.
 */
void InsertHandler(eventid_t id) {
    FRESULT err;

    (void)id;

    /*
     * On insertion SDC initialization and FS mount.
     */
    if (sdcConnect(&SDCD1))
        return;

    err = f_mount(0, &SDC_FS);
    if (err != FR_OK) {
        sdcDisconnect(&SDCD1);
        return;
    }
    fs_ready = TRUE;
}

/*!
 *  \brief Card removal event.
 */
void RemoveHandler(eventid_t id) {
    (void)id;
    sdcDisconnect(&SDCD1);
    fs_ready = FALSE;
}


FRESULT scan_files(BaseSequentialStream *chp, char *path) {
    FRESULT res;
    FILINFO fno;
    DIR dir;
    int i;
    char *fn;

#if _USE_LFN
    fno.lfname = 0;
    fno.lfsize = 0;
#endif
    res = f_opendir(&dir, path);
    if (res == FR_OK) {
        i = strlen(path);
        for (;;) {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0)
                break;
            if (fno.fname[0] == '.')
                continue;
            fn = fno.fname;
            if (fno.fattrib & AM_DIR) {
                path[i++] = '/';
                strcpy(&path[i], fn);
                res = scan_files(chp, path);
                if (res != FR_OK)
                    break;
                path[--i] = 0;
            }
            else {
                chprintf(chp, "%s/%s\r\n", path, fn);
            }
        }
    }
    return res;
}

//! @}

