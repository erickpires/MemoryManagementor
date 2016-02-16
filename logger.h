#ifndef LOGGER_H
#define LOGGER_H 1

typedef unsigned int uint;
typedef int bool;
#define FALSE 0
#define TRUE 1

typedef enum {
	log_process_created,
	log_process_ended,
	log_page_requested,
	log_page_swapedout,
	log_page_fault
} log_event;

typedef struct {
	uint process_id;
	log_event event;
	uint event_data;
	clock_t when;
} log_info;

#define MAX_LOG_ENTRIES 4096

bool log_(uint, log_event, uint);
void flush_log(void);

#endif
