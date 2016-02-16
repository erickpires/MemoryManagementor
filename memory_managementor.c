#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h>
#include "logger.h"
#include "memory_managementor.h"

#define has_free_frame() (free_frames != NULL)

static page_table page_tables[MAX_THREADS] = {};

static frame_list_node* free_frames;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void init_m_managementor() {

	free_frames = (frame_list_node*) malloc(sizeof(frame_list_node));
	free_frames->frame = 0;
	frame_list_node* current_frame = free_frames;

	for(uint frame = 1; frame < MAX_FRAMES; frame++) {
		current_frame->next = (frame_list_node*) malloc(sizeof(frame_list_node));
		current_frame->next->frame = frame;
		current_frame = current_frame->next;
	}

	printf("Size of tables %ld\n", sizeof(page_tables));
}

void m_managementor_get_page(uint id, uint page_number) {
	//page_info result = {};
	pthread_mutex_lock(&mutex);
	log_(id, log_page_requested, page_number);

	page_table* table = page_tables + id;
	page_info page = table->pages[page_number];
	if(page.is_available) {
		printf("Page is avaible\n");
		update_LRU(table, page_number);
	}
	else {
		if(table->allocated_pages_number < WORKING_SET_LIMIT) {
			printf("Working set is not full\n");
			if(has_free_frame()) {
				printf("Has free frame\n");
				uint frame = remove_free_frame();
				page.is_available = TRUE;
				page.frame = frame;
				insert_LRU(table, page_number);
			}
			else { // Do not have enough memory
				// swap_out_process
				printf("Swapout\n");

				// loop in page_table* table looking for the oldest process using most_recent_page_time
				uint oldest_page = clock(); // no process is older than clock()
				int oldest_process;

				for(int i = 0; i < MAX_THREADS; i++) {

					//page_table* used_page = page_tables[i]

					if(page_tables[i].most_recent_page_time < oldest_page){
						oldest_page = page_tables[i].most_recent_page_time; // new oldest most recent page
						oldest_process = i; // the proud owner of the oldest most recent page
					}
				}

				// now we know who to retire
				swap_out_process(oldest_process);

				// adding the new one
				// aparently this 4 lines of code below are responsible for a segfault
				// uint frame = remove_free_frame();
				// page.is_available = TRUE;
				// page.frame = frame;
				// insert_LRU(table, page_number);
			}
		}
		else { // Working set is full
			printf("Working set is full\n");
			uint least_recent_page_number = table->LRU_pages[0];
			page_info least_recent_page = table->pages[least_recent_page_number];

			page.frame = least_recent_page.frame;
			page.is_available = TRUE;
			least_recent_page.is_available = FALSE;

			replace_LRU(table, page_number);
		}
	}

	printf("Execution Unity %d requested page: %d\n", id, page_number);
	pthread_mutex_unlock(&mutex);
	return;// result;
}

void insert_LRU(page_table* table, uint page_number) {
	uint index = table->allocated_pages_number++;
	table->LRU_pages[index] = page_number;

	//Updates the age of the lastest page
	table->most_recent_page_time = clock();
}

void update_LRU(page_table* table, uint recent_page_number) {
	uint index;
	// Finds the index of the current page in the LRU list
	for(index = 0; index < WORKING_SET_LIMIT; index++) {
		if(table->LRU_pages[index] == recent_page_number) break;
	}

	// Continues from where the previous loop stopped, copying
	// each element to its right neighbor
	for(index++; index < WORKING_SET_LIMIT; index++) {
		table->LRU_pages[index - 1] = table->LRU_pages[index];
	}

	// Puts the current page in the rightmost position (most recent)
	table->LRU_pages[WORKING_SET_LIMIT - 1] = recent_page_number;

	//Updates the age of the lastest page
	table->most_recent_page_time = clock();

}

void replace_LRU(page_table* table, uint page_number) {
	for(uint index = 1; index < WORKING_SET_LIMIT; index++) {
		table->LRU_pages[index - 1] = table->LRU_pages[index];
	}
	table->LRU_pages[WORKING_SET_LIMIT] = page_number;

	//Updates the age of the lastest page
	table->most_recent_page_time = clock();
}

uint remove_free_frame() {
	uint result = free_frames->frame;
	frame_list_node* next = free_frames->next;

	free(free_frames);

	free_frames = next;
	return result;
}

void swap_out_process(int process) {

	for(int i = 0; i < MAX_THREADS; i++) {
		if(page_tables[process].pages[i].is_available) {
			// Give back the free frames
			frame_list_node*  new_free;
			new_free->frame = page_tables[process].pages[i].frame;
			new_free ->next = free_frames;
			free_frames = new_free;

			// the page isn't available anymore
			page_tables[process].pages[i].is_available = FALSE;

			//logging the swap out
			log_((uint)process,log_page_swapedout,(uint)i);
		}
	}

	// Clean the LRU
	for(int i = 0; i < WORKING_SET_LIMIT; i++) {
		page_tables[process].LRU_pages[i] = 0;
	}

	page_tables[process].allocated_pages_number = 0;
	//not resetting most_recent_page_time, don't need to
}
