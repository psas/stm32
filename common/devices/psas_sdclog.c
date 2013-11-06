/*! \file psas_sdclog.c
 *  SD Card Logging
 */

/*!
 * \defgroup psas_sdclog PSAS SDCard Logging Utilities
 * @{
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "chrtclib.h"
#include "psas_rtc.h"

#include "MPU9150.h"

#include "crc_16_reflect.h"
#include "ff.h"

#include "psas_sdclog.h"




#define         DEBUG_SDC

#ifdef DEBUG_SDC
    #include "usbdetail.h"
    BaseSequentialStream    *sdcchp   =  (BaseSequentialStream *)&SDU_PSAS;
    #define SDCDEBUG(format, ...) chprintf( sdcchp, format, ##__VA_ARGS__ )
#else
    #define SDCDEBUG(...) do{ } while ( false )
#endif


// static declarations
static const    unsigned        sdc_polling_interval             = 10;
static const    unsigned        sdc_polling_delay                = 10;

static          sdc_eod_marker  sdc_eod;

static          unsigned        sdc_debounce_count               = 0;

static          VirtualTimer    sdc_tmr;

// extern declarations
bool                            fs_ready                         = FALSE;

FATFS                           SDC_FS;

DWORD                           sdc_fp_index                     = 0;
DWORD                           sdc_fp_index_old                 = 0;

EventSource                     sdc_inserted_event;
EventSource                     sdc_removed_event;



/*
 * Function Definitions
 * ====================
 */

/*!
 * @brief           Insertion monitor timer callback function.
 *
 * @param[in] p     pointer to the @p BaseBlockDevice object
 *
 * @notapi
 */
static void sdc_tmrfunc(void *p) {
    BaseBlockDevice *bbdp = p;

    chSysLockFromIsr();
    if (sdc_debounce_count > 0) {
        if (blkIsInserted(bbdp)) {
            if (--sdc_debounce_count == 0) {
                chEvtBroadcastI(&sdc_inserted_event);
            }
        }
        else
            sdc_debounce_count = sdc_polling_interval;
    }
    else {
        if (!blkIsInserted(bbdp)) {
            sdc_debounce_count = sdc_polling_interval;
            chEvtBroadcastI(&sdc_removed_event);
        }
    }
    chVTSetI(&sdc_tmr, MS2ST(sdc_polling_delay), sdc_tmrfunc, bbdp);
    chSysUnlockFromIsr();
}

/*! \brief Init end of data fiducials.
 */
void sdc_init_eod(uint8_t marker_byte) {
    unsigned int i;

    for (i = 0; i < sizeof(sdc_eod_marker); i++) {
        sdc_eod.sdc_eodmarks[i] = marker_byte;
    }
}

/**
 * @brief   Polling monitor start.
 *
 * @param[in] p         pointer to an object implementing @p BaseBlockDevice
 *
 * @notapi
 */
void sdc_tmr_init(void *p) {

    chEvtInit(&sdc_inserted_event);
    chEvtInit(&sdc_removed_event);
    chSysLock();
    sdc_debounce_count = sdc_polling_interval;
    chVTSetI(&sdc_tmr, MS2ST(sdc_polling_delay), sdc_tmrfunc, p);
    chSysUnlock();
}

/*!
 * \brief Card insertion event.
 *
 * Also use at power up to see if card was or remained
 * inserted while power unavailable.
 */
void sdc_insert_handler(eventid_t id) {
    FRESULT err;

    (void)id;

    /*! \todo generate a mailbox event here */

    /*! \todo test event message system */

    /*!
     * On insertion SDC initialization and FS mount.
     */
    if (sdcConnect(&SDCD1)) {
        if(sdcConnect(&SDCD1)) {   // why does it often fail the first time but not the second?
            return;
        }
    }

    err = f_mount(0, &SDC_FS);
    if (err != FR_OK) {
        err = f_mount(0, &SDC_FS);
        if (err != FR_OK) {
            sdcDisconnect(&SDCD1);
            return;
        }
    }
    sdc_reset_fp_index();
    fs_ready = TRUE;
}

/*!
 *  \brief Card removal event.
 */
void sdc_remove_handler(eventid_t id) {
    (void)id;
    bool_t ret;

    /*! \todo generate a mailbox event here */

    /*! \todo test event message system */

    ret = sdcDisconnect(&SDCD1);
    if(ret) {
        SDCDEBUG("sdcDiscon fail\r\n");   // this happens a lot!
        ret = sdcDisconnect(&SDCD1);
        if(ret) {
            SDCDEBUG("sdcDiscon fail2\r\n");
        }
    }
    sdc_reset_fp_index();
    fs_ready = FALSE;
}

/*! \brief used in cmd_tree function
 *
 * Find files on sd card.
 */
FRESULT sdc_scan_files(BaseSequentialStream *chp, char *path) {
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
                res = sdc_scan_files(chp, path);
                if (res != FR_OK)
                    break;
                path[--i] = 0;
            }
            else {
                SDCDEBUG("%s/%s\r\n", path, fn);
            }
        }
    }
    return res;
}

/*! \brief Set the current filepointer (fp) index
 *
 *  sdcp_fp_index tracks what byte we are pointing to
 *  from start of file (byte0)
 */
void sdc_set_fp_index(FIL* DATAFil, DWORD ofs) {
    f_lseek(DATAFil, ofs);
    sdc_fp_index      = ofs;
}

/*! \brief Store a checksum and end of data fiducial marks
 *
     A line of data would look like this:
        [GenericMessage][chksum]
     We want a line at the end-of-data(eod) to look like this:
        [GenericMessage][chksum][0xa5a5]

     What happens if power fails partway through a write? There will
     be NO eod marker. So write a length of eod markers every write past the
     length of the next write.
        [GenericMessage][checksum][0xa5a5..(len(GenericMessage+checksum+some extra)..0xa5a5]
                                   ^
                                   Seek fp-index (sdc_set_fp_index) to here.

     Now, in the event of failure during the next write, we have already placed
     eod markers. A partial filled line will have an incorrect checksum.

     If we pre-allocate the file on the sd card, then the file will already exist
     in the FAT and shouldn't be damaged due to a partial file size reallocation.

 */
FRESULT sdc_write_checksum(FIL* DATAFil, const crc_t* crcd, uint32_t* bw) {
    FRESULT rc;

    if((crcd==NULL) || (bw == NULL)) {
        return SDC_NULL_PARAMETER_ERROR;
    }
    rc = f_write(DATAFil, (const void *)(crcd), sizeof(crc_t), (unsigned int*) bw);
    if (rc)  {
        SDCDEBUG("f_write ckdata error: %d\r\n", rc) ;
        return rc;
    }
    sdc_fp_index += *bw;

    rc = f_write(DATAFil, (const void *)(&sdc_eod), sizeof(sdc_eod_marker), (unsigned int*) bw);
    if (rc)  {
        SDCDEBUG("f_write eod error: %d\r\n", rc) ;
        return rc;
    }

    rc = f_sync(DATAFil);

    // be kind, rewind.
    sdc_set_fp_index(DATAFil, sdc_fp_index) ;

    rc = f_sync(DATAFil);
    if (rc)  {
        SDCDEBUG("f_sync error: %d\r\n", rc);
        return rc;
    }
    return FR_OK;
}

/*! \brief Store GenericMessage to the SD card
*/
FRESULT sdc_write_log_message(FIL* DATAFil, GenericMessage* d, uint32_t* bw) {
    FRESULT rc;

    if((d==NULL) || (bw == NULL)) {
        return SDC_NULL_PARAMETER_ERROR;
    }

    rc = f_write(DATAFil, (const void *)(d), sizeof(GenericMessage), (unsigned int*) bw);
    if (rc)  {
        SDCDEBUG("f_write error: %d\r\n", rc) ;
        return rc;
    }
    sdc_fp_index += *bw;

    rc = f_sync(DATAFil);
    if (rc)  {
        SDCDEBUG("%s: f_sync error: %d\r\n", __func__, rc);
        return rc;
    }
    return FR_OK;
}

/*!
 * Instead of restarting from byte0 in opened existing data file, continue
 * from last successful data write.
 *
 * Track from reset due to power cycle or watchdog for instance.
 *
 */
void sdc_seek_eod(FIL* DATAFil, GenericMessage* d, uint32_t* sdindexbyte) {
    (void) DATAFil;     // temporary, these are void for compile purposes.
    (void) d;           // ditto
    (void) sdindexbyte; // ibid

    SDCDEBUG("%s: Not implemented yet.\r\n", __func__);
    /*! step 1: Use eeprom (not avail on e407) as scratch pad to end of data. */
    /*! step 2: Confirm end of data by finding last data with successful checksum */
}


//! @}

