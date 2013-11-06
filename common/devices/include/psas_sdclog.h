/*! \file psas_sdclog.h
 *
 */

#ifndef PSAS_SDCLOG_H_
#define PSAS_SDCLOG_H_

/*!
 * \addtogroup psas_sdclog
 * @{
 */
#include "ch.h"
#include "hal.h"
#include "ff.h"

#include "psas_rtc.h"
#include "usbdetail.h"

#include "crc_16_reflect.h"

#ifdef __cplusplus
extern "C" {
#endif



/*
 * Definitions
 * =========== *****************************************************************
 */

#define SDC_MSG_ID_BYTES            4
#define SDC_MSG_HEAD_BYTES          16
#define SDC_MSG_MAX_PAYLOAD_BYTES   150
#define SDC_MSG_CHECKSUM_BYTES      2
#define SDC_MSG_EOD_BYTES           1
#define SDC_MSG_EOD_INSTANCES       2



/*
 * Global Variables
 * ================ ************************************************************
 */

extern  bool        fs_ready;
extern  FATFS       SDC_FS;
extern  DWORD       sdc_fp_index;
extern  DWORD       sdc_fp_index_old;

extern  EventSource sdc_inserted_event;
extern  EventSource sdc_removed_event;



/*
 * Typedefs
 * ======== ********************************************************************
 */

// create end of data fiducials...slightly larger than log entry
typedef struct sdc_eod_marker {
    uint8_t sdc_eodmarks[SDC_MSG_HEAD_BYTES
                         + SDC_MSG_MAX_PAYLOAD_BYTES
                         + SDC_MSG_CHECKSUM_BYTES
                         + (SDC_MSG_EOD_BYTES * SDC_MSG_EOD_INSTANCES)
                        ];
} sdc_eod_marker;

typedef enum SDC_ERRORCODE {
    SDC_OK                   = 0,
    SDC_NULL_PARAMETER_ERROR = -1,
    SDC_FSYNC_ERROR          = -2,
    SDC_FWRITE_ERROR         = -3,
    SDC_ASSERT_ERROR         = -4,
    SDC_UNKNOWN_ERROR        = -99
} SDC_ERRORCODE;

typedef struct MessageHead {
    char            ID[SDC_MSG_ID_BYTES]; // This must be first part of data. Reserved value: 0xa5a5
    uint32_t        index;
    psas_timespec   ts;
    uint16_t        data_length;
} __attribute__((packed)) MessageHead;

typedef struct GenericMessage {
    MessageHead head;                             // 16 bytes
    uint8_t     data[SDC_MSG_MAX_PAYLOAD_BYTES];  // 150 bytes
} __attribute__((packed)) GenericMessage;



/*
 * Function Declarations
 * ===================== *******************************************************
 */

void    sdc_insert_handler(eventid_t id);
void    sdc_remove_handler(eventid_t id);
void    sdc_tmr_init(void *p);
void    sdc_set_fp_index(FIL* data_file, DWORD ofs);
void    sdc_init_eod(uint8_t marker_byte);

FRESULT sdc_write_checksum(FIL* data_file, const crc_t* d, uint32_t* bw);
FRESULT sdc_write_log_message(FIL* data_file, GenericMessage* d, uint32_t* bw);
FRESULT sdc_scan_files(BaseSequentialStream* chp, char *path);

/*! \todo implement sdc_seek_eod function. */
void    sdc_seek_eod(FIL* DATAFil, GenericMessage* d, uint32_t* sdindexbyte) ;



/*
 * Inline Function Definitions
 * =========================== *************************************************
 */

static inline void sdc_reset_fp_index(void) {
    sdc_fp_index = 0;
}



#ifdef __cplusplus
}
#endif
//! @}

#endif

