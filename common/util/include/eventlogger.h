/*!
 * \file eventlogger.h
 */

/*
 * Header Boilerplate
 * ================== **********************************************************
 */

#ifndef PSAS_EVENTLOGGER_H_
#define PSAS_EVENTLOGGER_H_

#ifdef __cplusplus
extern "C" {
#endif



/*
 * API Functions
 * ============= ***************************************************************
 */

void eventlogger_init(void);
bool log_event(const char* id, const uint8_t* data, uint16_t data_length);



/*
 * More Header Boilerplate
 * ======================= *****************************************************
 */

#ifdef __cplusplus
}
#endif

// this closes the #ifndef PSAS_EVENTLOGGER_H_ conditional
#endif
