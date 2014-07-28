#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "chrtclib.h"

#include "psas_sdclog.h"
#include "usbdetail.h"

#include "eventlogger.h"



/*
 * Definitions
 * =========== *****************************************************************
 */

#define UNUSED __attribute__((unused))

#define EVENTBUFF_LENGTH 64
#define EVENTLOG_DEBUG true

#ifdef EVENTLOG_DEBUG
#define LOG_DEBUG(format, ...) chprintf(getUsbStream(), format, ##__VA_ARGS__ )
#else
#define LOG_DEBUG(...)
#endif

static const char* sdc_log_file    = "LOGSMALL.bin";



/*
 * Private Function Declarations
 * ============================= ***********************************************
 */

/*
 * This is the logger thread that writes the posted events to the SD card.
 */
static msg_t eventlogger(void* _);

/*
 * This creates a GENERIC_message struct with the given ID and data, filling out
 * all other requisite fields as appropriate.
 */
GENERIC_message* make_msg(const char* id, const uint8_t* data, uint16_t data_length);



/*
 * Private Global Variables
 * ======================== ****************************************************
 */

/*
 * This here mailbox is the hidden point of contact between the public
 * log_event() function and the private log_event().
 */
static msg_t mail_buffer[EVENTBUFF_LENGTH];
static MAILBOX_DECL(event_mail, mail_buffer, EVENTBUFF_LENGTH);

static MemoryPool msg_pool;
/*
 * This is the static storage that backs the GENERIC_message memory pool.
 * The size of this array defines the maximum number of GENERIC_message objects
 * that can be allocated at any one time.
 * Since each message posted to the above mailbox will refer to one
 * GENERIC_message, this array should have the same size as the mailbox.
 */
static GENERIC_message msg_data[EVENTBUFF_LENGTH] __attribute__((aligned(sizeof(stkalign_t))));

static WORKING_AREA(wa_thread_eventlogger, 2048);



/*
 * Public Function Definitions
 * =========================== *************************************************
 */

/*
 * log_event
 *
 * This is how you send things to the logger so it can log them.
 * The id parameter must point to an array of at least SDC_MSG_ID_BYTES chars.
 * The data_length parameter must accurately represent the length of the array
 * pointed to by the data parameter, and this length must be no longer than
 * SDC_MSG_MAX_PAYLOAD_BYTES. The returned boolean indicates whether the event
 * was succesfully posted (true) or failed because the buffer of events to be
 * logged was full (false).
 */
bool log_event(const char* id, const uint8_t* data, uint16_t data_length) {
	msg_t status;

	GENERIC_message* msg = make_msg(id, data, data_length);
	if (msg == NULL) return false;

	status = chMBPost(&event_mail, (msg_t) msg, TIME_IMMEDIATE);

	return status == RDY_OK;
}


/*
 * eventlogger_init
 *
 * This must be called before post_event can be used.
 * It sets up the logger thread and Event memory pool.
 * It assumes that ChibiOs's RTOS kernel has already been activated by calling
 * chSysInit().
 */
void eventlogger_init(void) {
	chPoolInit(&msg_pool, sizeof(GENERIC_message), NULL);
	chPoolLoadArray(&msg_pool, msg_data, EVENTBUFF_LENGTH);

	chThdCreateStatic( wa_thread_eventlogger
	                 , sizeof(wa_thread_eventlogger)
	                 , NORMALPRIO
	                 , eventlogger
	                 , NULL
	                 );
}



/*
 * Private Function Definitions
 * ============================ ************************************************
 */
/* Utility to convert nanoseconds to psas time. Should go somewhere else*/
static to_psas_time(uint8_t * ts, uint64_t time_ns){
	ts[0] = time_ns >> 40;
	ts[1] = time_ns >> 32;
	ts[2] = time_ns >> 24;
	ts[3] = time_ns >> 16;
	ts[4] = time_ns >> 8;
	ts[5] = time_ns >> 0;
}

/*
 * eventlogger
 *
 * Log messages from the event mailbox to the SD card.
 */
static msg_t eventlogger(void *_ UNUSED) {
	uint32_t        write_errs = 0;
	uint32_t        bytes_written;
	bool            log_opened = false;
	FIL             LogFile;
	GENERIC_message* posted;

	chRegSetThreadName("eventlogger");
	LOG_DEBUG("Started eventlog thread\r\n");

	sdc_reset_fp_index();
	sdc_init_eod((uint8_t) 0xa5);

	// ensure message is half-word aligned (i.e. 16-bit aligned)
	if (sizeof(GENERIC_message) % 2 != 0) {
		LOG_DEBUG("Generic message is not 16-bit aligned!\r\n");
		return SDC_ASSERT_ERROR;
	}

	while (true) {

		// if sd card has not been mounted, sleep for 10 ms then try again
		if (!fs_ready && !log_opened) {
			chThdSleepMilliseconds(10);
			continue;
		}

		// if sd card has been mounted but the log file hasn't been opened, try
		// to open it.
		if (fs_ready && !log_opened) {
			FRESULT file_err;

			int tries = 0;
			do {
				file_err = f_open(&LogFile, sdc_log_file, FA_OPEN_EXISTING | FA_WRITE);
				tries++;
			} while (file_err && tries < 3);

			// if we couldn't open the log file, try to create a new one
			if (file_err) {
				LOG_DEBUG("Failed to open existing log file \"%s\"\r\n", sdc_log_file);

				tries = 0;
				do {
					file_err = f_open(&LogFile, sdc_log_file, FA_CREATE_ALWAYS | FA_WRITE);
					tries++;
				} while (file_err && tries < 3);

				log_opened = !file_err;

				if (log_opened) {
					LOG_DEBUG("Created new log file \"%s\"\r\n", sdc_log_file);
				} else {
					LOG_DEBUG("Failed to create new log file \"%s\"\r\n", sdc_log_file);
				}
			} else {
				LOG_DEBUG("Opened existing log file \"%s\"\r\n", sdc_log_file);
				log_opened = true;
			}

			continue;
		}

		// if sd card is not mounted but our log is open (meaning the card was
		// mounted previously and has since been unmounted), close the log
		if (!fs_ready && log_opened) {
			int status = f_close(&LogFile);
			LOG_DEBUG("close log file status: %d\r\n", status);
			log_opened = false;

			continue;
		}

		// if sd card has been mounted and the log file has been opened, wait
		// for a message from our mailbox and log it to disk.
		if (fs_ready && log_opened) {
			int           i;
			int           status;
			crc_t         crc16;
			psas_timespec logged_ts;
			uint64_t      posted_ns = 0;
			uint64_t      ns_delay = 0;

			chMBFetch(&event_mail, (msg_t *) &posted, TIME_INFINITE);
			status = sdc_write_log_message(&LogFile, posted, &bytes_written);

			if (status != FR_OK) {
				write_errs++;
				LOG_DEBUG( "Could not log message %5d: error %d\r\n"
				         , posted->mh.index
				         , status
				         );
				goto msg_cleanup;
			}

			crc16 = crc_init();
			crc16 = crc_update(crc16, (const unsigned char*) posted, sizeof(GENERIC_message));
			crc16 = crc_finalize(crc16);

			status = sdc_write_checksum(&LogFile, &crc16, &bytes_written);
			if (status != FR_OK) {
				write_errs++;
				LOG_DEBUG( "Could not write checksum for message %5d: error %d\r\n"
				         , posted->mh.index
				         , status
				         );
				goto msg_cleanup;
			}

			// calculate time between when the message was created and when we
			// logged it, in ns
			uint64_t now = rtcGetTimeUsec(&RTCD1);
			to_psas_time(logged_ts.ns, now*1000);

			for (i = 0; i < sizeof(logged_ts.ns); i++) {
				posted_ns += (uint64_t) posted->mh.ts.ns[i] << (i * 8);
				ns_delay += (uint64_t) logged_ts.ns[i] << (i * 8);
			}
			ns_delay -= posted_ns;

			LOG_DEBUG( "Logged message %5u with delay of %3u.%06u ms\r\n"
			         , posted->mh.index
			         , (uint32_t) (ns_delay / 1000000)
			         , (uint32_t) (ns_delay % 1000000)
			         );

msg_cleanup:
			chPoolFree(&msg_pool, posted);
			continue;
		}
	}

	return -1;
}


GENERIC_message* make_msg(const char* id, const uint8_t* data, uint16_t data_length) {
	static int msg_index = 0;

	GENERIC_message* msg = (GENERIC_message*) chPoolAlloc(&msg_pool);

	// bail if the pool is empty
	if (msg == NULL) return NULL;

	// fill out most of message's head fields
	strncpy(msg->mh.ID, id, SDC_NUM_ID_CHARS );
	msg->mh.index = msg_index++;

	// fill out message's head timestamp field
	uint64_t now = rtcGetTimeUsec(&RTCD1);
	to_psas_time(msg->mh.ts.ns, now*1000);

	// copy the data into the msg
	msg->mh.data_length = data_length;
	memcpy(&msg->data, data, data_length);

	return msg;
}
