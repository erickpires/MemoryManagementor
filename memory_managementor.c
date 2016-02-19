#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include "logger.h"
#include "memory_managementor.h"

#define has_free_frame() (free_frames != NULL)

static page_table page_tables[MAX_THREADS] = {};

static frame_list_node* free_frames;

static pthread_mutex_t disk_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t frames_mutex = PTHREAD_MUTEX_INITIALIZER;

inline void wait_clocks(clock_t clocks) {
	clock_t start_clock = clock();
	clock_t current_clock;
	do {
		current_clock = clock();
	} while(current_clock - start_clock < clocks);
}

void init_m_managementor() {
	free_frames = (frame_list_node*) malloc(sizeof(frame_list_node));
	free_frames->frame = 0;
	frame_list_node* current_frame = free_frames;

	for(uint process = 0; process < MAX_THREADS; process++) {
		pthread_mutex_init(&(page_tables[process].LRU_mutex), NULL);
	}

	for(uint frame = 1; frame < MAX_FRAMES; frame++) {
		current_frame->next = (frame_list_node*) malloc(sizeof(frame_list_node));
		current_frame->next->frame = frame;
		current_frame = current_frame->next;
	}
}

void m_managementor_get_page(uint id, uint page_number) {
	//page_info result = {};
	// TODO(log): alocação de memória real de acordo com a solicitação das páginas
	// TODO(log): lista de substituição de páginas (LRU)
	// TODO: A cada solicitação de página o gerenciador da MV tem que apresentar a tabela de páginas virtuais do processo solicitante;
	log_(id, log_page_requested, &page_number);

	page_table* table = page_tables + id;
	page_info* page = table->pages + page_number;
	if(page->is_available) {
		update_LRU(id, page_number);
	}
	else {
		log_(id, log_page_fault, &page_number);

		pthread_mutex_lock(&disk_mutex);
		wait_clocks(CLOCKS_FOR_DISK_ACCESS);
		pthread_mutex_unlock(&disk_mutex);

		if(table->allocated_pages_number < WORKING_SET_LIMIT) {
			pthread_mutex_lock(&frames_mutex);
			if(has_free_frame()) {
				uint frame = remove_free_frame();

				pthread_mutex_unlock(&frames_mutex);

				page->is_available = TRUE;
				page->frame = frame;
				insert_LRU(id, page_number);
			}
			else { // Do not have enough memory
				// swap_out_process

				// loop in page_table* table looking for the oldest process using most_recent_page_time
				clock_t oldest_page = clock(); // no process is older than clock()
				uint oldest_process;

				for(int current_process = 0; current_process < MAX_THREADS; current_process++) {
					//page_table* used_page = page_tables[i]
					if(page_tables[current_process].allocated_pages_number > 0 &&
					   page_tables[current_process].most_recent_page_time < oldest_page) {
						oldest_page = page_tables[current_process].most_recent_page_time; // new oldest most recent page
						oldest_process = current_process; // the proud owner of the oldest most recent page
					}
				}

				// now we know who to retire
				swap_out_process(oldest_process);

				// adding the new one
				uint frame = remove_free_frame();

				pthread_mutex_unlock(&frames_mutex);

				page->is_available = TRUE;
				page->frame = frame;
				insert_LRU(id, page_number);
			}
		}
		else { // Working set is full
			uint least_recent_page_number = table->LRU_pages[0];
			page_info* least_recent_page = table->pages + least_recent_page_number;

			page->frame = least_recent_page->frame;
			page->is_available = TRUE;
			least_recent_page->is_available = FALSE;

			replace_LRU(id, page_number);
		}
	}

	// printf("Execution Unity %d requested page: %d\n", id, page_number);

	log_(id, log_page_delivered, table);
	log_(id, log_frames_updated, page_tables);
	return;// result;
}

void insert_LRU(uint process_id, uint page_number) {
	page_table* table = page_tables + process_id;
	pthread_mutex_lock(&(table->LRU_mutex));
	log_(process_id, log_LRU_will_change, table);

	uint index = table->allocated_pages_number++;
	table->LRU_pages[index] = page_number;

	// Updates the age of the last page
	table->most_recent_page_time = clock();
	log_(process_id, log_LRU_changed, table);
	pthread_mutex_unlock(&(table->LRU_mutex));
}

void update_LRU(uint process_id, uint recent_page_number) {
	page_table* table = page_tables + process_id;
	pthread_mutex_lock(&(table->LRU_mutex));
	log_(process_id, log_LRU_will_change, table);

	uint index;
	// Finds the index of the current page in the LRU list
	for(index = 0; index < table->allocated_pages_number; index++) {
		if(table->LRU_pages[index] == recent_page_number) break;
	}

	// Continues from where the previous loop stopped, copying
	// each element to its right neighbor
	for(index++; index < table->allocated_pages_number; index++) {
		table->LRU_pages[index - 1] = table->LRU_pages[index];
	}

	// Puts the current page in the rightmost position (most recent)
	table->LRU_pages[table->allocated_pages_number - 1] = recent_page_number;

	// Updates the age of the last page
	table->most_recent_page_time = clock();

	log_(process_id, log_LRU_changed, table);
	pthread_mutex_unlock(&(table->LRU_mutex));
}

void replace_LRU(uint process_id, uint page_number) {
	page_table* table = page_tables + process_id;
	pthread_mutex_lock(&(table->LRU_mutex));
	log_(process_id, log_LRU_will_change, table);

	for(uint index = 1; index < WORKING_SET_LIMIT; index++) {
		table->LRU_pages[index - 1] = table->LRU_pages[index];
	}
	table->LRU_pages[WORKING_SET_LIMIT - 1] = page_number;

	// Updates the age of the last page
	table->most_recent_page_time = clock();

	log_(process_id, log_LRU_changed, table);
	pthread_mutex_unlock(&(table->LRU_mutex));
}

uint remove_free_frame() {
	uint result = free_frames->frame;
	frame_list_node* next = free_frames->next;

	free(free_frames);

	free_frames = next;
	return result;
}

void swap_out_process(uint process) {
	pthread_mutex_lock(&disk_mutex);
	wait_clocks(CLOCKS_FOR_DISK_ACCESS);
	pthread_mutex_unlock(&disk_mutex);

	page_table* table = page_tables + process;

	pthread_mutex_lock(&(table->LRU_mutex));

	for(uint i = 0; i < MAX_PAGES; i++) {
		if(table->pages[i].is_available) {
			// Give back the free frames
			frame_list_node*  new_free = (frame_list_node*) malloc(sizeof(frame_list_node));
			new_free->frame = page_tables[process].pages[i].frame;
			new_free->next = free_frames;
			free_frames = new_free;
			// the page isn't available anymore
			page_tables[process].pages[i].is_available = FALSE;

			printf("%d ", page_tables[process].pages[i].frame);
			// logging the swap out
			log_(process, log_page_swapedout, &i);
		}
	}

	page_tables[process].allocated_pages_number = 0;
	pthread_mutex_unlock(&(table->LRU_mutex));
}
