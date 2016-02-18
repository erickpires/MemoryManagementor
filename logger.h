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
	log_page_delivered,
	log_LRU_will_change,
	log_LRU_changed,
	log_frames_updated,
	log_page_swapedout,
	log_page_fault
} log_event;

typedef struct {
	uint process_id;
	log_event event;
	void* event_data;
	double when;
} log_info;

typedef struct {
	uint process_id;
	uint page_number;
	bool is_allocated;
} frame_info;

#define MAX_LOG_ENTRIES 4096

void init_logger(void);
bool log_(uint, log_event, void*);
// void flush_log(void);
void treat_log_info(log_info*);
void print_page_table(page_table*);
void print_LRU(page_table*);
void print_main_memory(page_table*);

#endif
