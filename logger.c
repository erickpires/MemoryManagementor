#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "work_thread.h"
#include "memory_managementor.h"
#include "logger.h"

bool log_(uint process_id, log_event event, void* event_data) {
	log_info info = {};
	clock_t now = clock();
	info.process_id = process_id;
	info.event = event;
	info.event_data = event_data;
	info.when = now;

	treat_log_info(&info);

	return TRUE;
}

// void flush_log() {
// 	for(uint i = 0; i < current_log_entry; i++) {
// 		log_info* info = log_data + i;
// 		treat_log_info(info);
// }

void treat_log_info(log_info* info) {
	uint* uint_event_data = (uint*) info->event_data;
	switch(info->event) {
		case log_process_created :
			printf("Process %d created at %ld\n", info->process_id, info->when);
			break;
		case log_process_ended :
			printf("Process %d ended at %ld\n", info->process_id, info->when);
			break;
		case log_page_requested :
			printf("Process %d requested page %d at %ld\n", info->process_id, *uint_event_data, info->when);
			break;
		case log_page_swapedout :
			printf("Page %d of process %d swaped out at %ld\n", *uint_event_data, info->process_id, info->when);
			break;
		case log_page_fault :
			printf("Pagefault in page %d of process %d at %ld\n", info->process_id, *uint_event_data, info->when);
			break;
		case log_page_delivered :
			printf("Virtual page table of process %d\n", info->process_id);
			print_page_table((page_table*) info->event_data);
			break;
		case log_LRU_will_change :
			printf("Old LRU of process %d:\n", info->process_id);
			print_LRU((page_table*) info->event_data);
			break;
		case log_LRU_changed :
			printf("New LRU of process %d:\n", info->process_id);
			print_LRU((page_table*) info->event_data);
			break;
		case log_frames_updated :
			printf("Main memory:\n");
			print_main_memory((page_table*) info->event_data);
			break;
		default :
			break;
	}
}

void print_page_table(page_table* table) {
	page_info* pages = table->pages;
	printf("| PAGE# | IN_MEMORY | FRAME# |\n");
	for(uint page_index = 0; page_index < MAX_PAGES; page_index++) {
		if(pages[page_index].is_available)
			printf("|  %2d   |    YES    |   %2d   |\n", page_index, pages[page_index].frame);
		else
			printf("|  %2d   |     NO    |   --   |\n", page_index);
	}
}

void print_LRU(page_table* table) {
	printf("[ ");
	for(uint index = 0; index < table->allocated_pages_number; index++) {
		printf("%d ", table->LRU_pages[index]);
	}
	printf("]\n");
}

void print_main_memory(page_table* tables) {
	frame_info frames[MAX_FRAMES] = {};

	for(uint process_index = 0; process_index < MAX_THREADS; process_index++) {
		page_table* table = tables + process_index;

		for(uint LRU_page_index = 0; LRU_page_index < table->allocated_pages_number; LRU_page_index++) {
			uint page_index = table->LRU_pages[LRU_page_index];
			uint frame = table->pages[page_index].frame;

			frames[frame].process_id = process_index;
			frames[frame].page_number = page_index;
			frames[frame].is_allocated = TRUE;
		}
	}

	printf("| FRAME# | PROCESS# | PAGE# |\n");

	for(uint frame_index = 0; frame_index < MAX_FRAMES; frame_index++) {
		frame_info* current_frame = frames + frame_index;
		if(current_frame->is_allocated) {
			printf("|   %2d   |    %2d    |   %2d  |\n", frame_index, current_frame->process_id, current_frame->page_number);
		}
	}
}
