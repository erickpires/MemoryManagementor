#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "logger.h"

static uint current_log_entry = 0;
static log_info log_data[MAX_LOG_ENTRIES] = {};

bool log_(uint process_id, log_event event, uint event_data) {
	if(current_log_entry == MAX_LOG_ENTRIES)
		return FALSE;

	log_info* info = log_data + current_log_entry++;
	clock_t now = clock();
	info->process_id = process_id;
	info->event = event;
	info->event_data = event_data;
	info->when = now;

	return TRUE;
}

void flush_log() {
	for(uint i = 0; i < current_log_entry; i++) {
		log_info info = log_data[i];
		switch(info.event) {
			case log_process_created :
				printf("Process %d created at %ld\n", info.process_id, info.when);
				break;
			case log_process_ended :
				printf("Process %d ended at %ld\n", info.process_id, info.when);
				break;
			case log_page_requested :
				printf("Process %d requested page %d at %ld\n", info.process_id, info.event_data, info.when);
				break;
			case log_page_swapedout :
				printf("Page %d of process %d swaped out at %ld\n", info.event_data, info.process_id, info.when);
				break;
			case log_page_fault :
				printf("Pagefault in page %d of process %d at %ld\n", info.process_id, info.event_data, info.when);
				break;
		}
	}
}
