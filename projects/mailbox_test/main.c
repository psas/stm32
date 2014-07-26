#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "utils_general.h"
#include "usbdetail.h"


#define MB_SIZE 32

BaseSequentialStream * chp = NULL;

msg_t mail_buffer[MB_SIZE];

static MAILBOX_DECL(my_mail, mail_buffer, MB_SIZE);

static bool UNUSED mail_singlethread_test(void) {
	chMBPost(&my_mail, 'A', TIME_IMMEDIATE);
	msg_t fetched_msg;
	msg_t fetch_status = chMBFetch(&my_mail, &fetched_msg, TIME_IMMEDIATE);

	if (fetch_status != RDY_OK) {
		// figure out how to print out an error here
		return false;
	}

	if (fetched_msg != 'A') {
		// figure out how to print out an error here
		return false;
	}

	return true;
}

static char TEST_MESSAGE[] = "HELLO WORLD!";
#define TEST_MB_MSG_LENGTH sizeof(TEST_MESSAGE)
static WORKING_AREA(waThread_Test_Sender, 64);

static msg_t mail_multithread_test_sender(void *_ UNUSED) {
	for (unsigned int i = 0; i < TEST_MB_MSG_LENGTH; i++) {
		//chprintf(chp, "sending test message #%d\r\n", i);
		if (chMBPost(&my_mail, TEST_MESSAGE[i], TIME_IMMEDIATE) != RDY_OK) {
			//chprintf(chp, "could not send test message #%d\r\n", i);
		}
	}

	return 0;
}

static char mail_test_buffer[TEST_MB_MSG_LENGTH];
static WORKING_AREA(waThread_Test_Receiver, 512);

static msg_t mail_multithread_test_receiver(void *_ UNUSED) {
	unsigned int i;

	for (i = 0; i < TEST_MB_MSG_LENGTH; i++) {
		chprintf(chp, "waiting for test message\r\n", i);
		chMBFetch(&my_mail, (msg_t *)&mail_test_buffer[i], TIME_INFINITE);
	}

	if(strncmp(mail_test_buffer, TEST_MESSAGE, TEST_MB_MSG_LENGTH) !=0) {
		chprintf(chp, "test message is incorrect!\r\n");
		return 1;
	}

	chprintf(chp, "all test messages received as expected!\r\n");

	return 0;
}

void main(void) {
	halInit();
	chSysInit();

	chp = getUsbStream();

	/*
	 * Spawn the test threads
	 */
	chThdCreateStatic( waThread_Test_Receiver
	                 , sizeof(waThread_Test_Receiver)
	                 , NORMALPRIO
	                 , mail_multithread_test_receiver
	                 , NULL
	                 );
	chThdCreateStatic( waThread_Test_Sender
	                 , sizeof(waThread_Test_Sender)
	                 , NORMALPRIO
	                 , mail_multithread_test_sender
	                 , NULL
	                 );

	while (TRUE) {
		chThdSleep(TIME_INFINITE);
	}
}

