/*
 * Types
 */

typedef uint8_t event_t;



/*
 * The API
 */

void eventlogger_init(void);
bool post_event(event_t et);
