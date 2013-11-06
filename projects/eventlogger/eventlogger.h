/*
 * The API
 */

void eventlogger_init(void);
bool log_event(const char* id, const uint8_t* data, uint16_t data_length);
