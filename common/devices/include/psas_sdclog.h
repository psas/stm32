/*! \file psas_sdclog.h
 * */

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


/* A 162 byte message written at 1000hz will use 4GB in about 6.5 Hours */
#define         SDC_MAX_PAYLOAD_BYTES                       150
#define         SDC_NUM_ID_CHARS                            4
#define         SDC_BOM_MARK                                0x5a5a
#define         SDC_MARKER_BYTES                            ((int) (SDC_MAX_PAYLOAD_BYTES + 50))


extern          bool            fs_ready;
extern          FATFS           SDC_FS;
extern          DWORD           sdc_fp_index;
extern          DWORD           sdc_fp_index_old;

extern          EventSource     sdc_inserted_event;
extern          EventSource     sdc_removed_event;
extern          EventSource     sdc_halt_event;
extern          EventSource     sdc_start_event;


// create end of data fiducials...slightly larger than log entry
typedef struct sdc_eod_marker {
	// GENERIC_message + checksum + eod_marker + eod_marker
	uint16_t marker;
	uint8_t sdc_eodmarks[SDC_MARKER_BYTES];
} sdc_eod_marker;

typedef enum SDC_ERRORCode {
	SDC_OK                   = 0,
	SDC_NULL_PARAMETER_ERROR = -1,
	SDC_FSYNC_ERROR          = -2,
	SDC_FWRITE_ERROR         = -3,
	SDC_SYNC_ERROR           = -4,
	SDC_FREAD_ERROR          = -5,
	SDC_ASSERT_ERROR         = -6,
	SDC_CHECKSUM_ERROR       = -7,
	SDC_FSEEK_ERROR          = -8,
	SDC_UNKNOWN_ERROR        = -99
} SDC_ERRORCode;

struct Message_head {
	char                 ID[SDC_NUM_ID_CHARS];         // This must be first part of data. Reserved value: 0xa5a5
	uint32_t             index;
	psas_timespec        ts;
	uint16_t             data_length;
} __attribute__((packed));
typedef struct Message_head Message_head;

/*! RTCLogtime is not psas_timespec.
 *      psas_timespec is time since FC reboot
 *      RTCLogtime    is the RTC on the STM32, running on a backup battery.
 */
struct RTCLogtime {
	time_t   tv_time;
	uint32_t tv_msec;
} __attribute__((packed));
typedef struct RTCLogtime RTCLogtime;

struct GENERIC_message {
	Message_head         mh;                           // 16 bytes
	RTCLogtime           logtime;                      // 12 bytes
	uint8_t              data[SDC_MAX_PAYLOAD_BYTES];  // 150 bytes
} __attribute__((packed));
typedef struct GENERIC_message GENERIC_message;

static inline void sdc_reset_fp_index(void) {
	sdc_fp_index = 0;
}

static inline DWORD sdc_get_fp_index(void) {
	return sdc_fp_index;
}

void            sdc_haltnow(void) ;
void            sdc_insert_handler(eventid_t id) ;
void            sdc_remove_handler(eventid_t id) ;
void            sdc_tmr_init(void *p) ;
SDC_ERRORCode   sdc_set_fp_index(FIL* DATAFil, DWORD ofs) ;
void            sdc_init_eod (uint8_t marker_byte) ;

SDC_ERRORCode   sdc_write_checksum(FIL* DATAFil, crc_t* crcd, uint32_t* bw) ;
SDC_ERRORCode   sdc_write_log_message(FIL* DATAFil, GENERIC_message* d, uint32_t * bw) ;
FRESULT         sdc_scan_files(BaseSequentialStream *chp, char *path) ;

SDC_ERRORCode sdc_f_write(FIL* fp, void* buff, unsigned int btr,  unsigned int*  bytes_written);
SDC_ERRORCode sdc_f_read(FIL* fp, void* buff, unsigned int btr,  unsigned int*  bytes_read) ;

SDC_ERRORCode sdc_seek_eod(FIL* DATAFil ) ;

#ifdef __cplusplus
}
#endif
//! @}

#endif
