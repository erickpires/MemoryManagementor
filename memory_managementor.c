#include <stdio.h>
#include <stdlib.h>
#include "logger.h"
#include "memory_managementor.h"

#define has_free_frame() (free_frames != NULL)

static page_table page_tables[MAX_THREADS] = {};

static frame_list_node* free_frames;

void init_m_managementor() {
	frame_list_node* current_frame = free_frames;
	for(int frame = 0; frame < MAX_FRAMES; frame++) {
		current_frame = (frame_list_node*) malloc(sizeof(frame_list_node));
		current_frame->frame = frame;
		current_frame = current_frame->next;
	}

	printf("Size of tables %ld\n", sizeof(page_tables));
}

void m_managementor_get_page(uint id, uint page_number) {
	//page_info result = {};

	log_(id, log_page_requested, page_number);

	page_table table = page_tables[id];
	page_info page = table.pages[page_number];
	if(page.is_available) {
		update_LRU(table, page_number);
	}
	else {
		if(table.allocated_pages_number < WORKING_SET_LIMIT) {
			if(has_free_frame()) {
				uint frame = remove_free_frame();
				page.is_available = TRUE;
				page.frame = frame;
				insert_LRU(table, page_number);
			}
			else { // Do not have enough memory
				// swap_out_process
			}
		}
		else { // Working set is full
			uint least_recent_page_number = table.LRU_pages[0];
			page_info least_recent_page = pages[least_recent_page_number];

			page.frame = least_recent_page.frame;
			page.is_available = TRUE;
			least_recent_page.is_available = FALSE;

			delete_and_insert_LRU(table, least_recent_page_number, page_number);
		}
	}

	printf("Execution Unity %d requested page: %d\n", id, page_number);
	return;// result;
}

