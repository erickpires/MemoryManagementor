#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "logger.h"
#include "work_thread.h"
#include "memory_managementor.h"

static bool threads_are_running = TRUE;

void* thread_code(void* _t_info) {
	thread_info* t_info = (thread_info*) _t_info;

	while(threads_are_running) {
		int random_page = rand() % MAX_PAGES;
		m_managementor_get_page(t_info->t_id, random_page);
		sleep(SLEEP_TIME);
	}
	log_(t_info->t_id, log_process_ended, 0);
	return NULL;
}

static pthread_t threads[MAX_THREADS] = {};
static thread_info t_infos[MAX_THREADS] = {};

void create_thread() {
	static uint current_thread_id = 0;

	thread_info* t_info = t_infos + current_thread_id;
	t_info->t_id = current_thread_id;
	pthread_create(threads + current_thread_id, NULL, thread_code, t_info);
	current_thread_id++;

	log_(t_info->t_id, log_process_created,0);
}

void join_threads() {
	pthread_t* thread_ptr = threads;
	for(int i = 0; i < MAX_THREADS; i++) {
		if(thread_ptr != NULL){
			pthread_join(*thread_ptr, NULL);
		}
		thread_ptr++;
	}
}

bool stop_threads() {
	return !(threads_are_running = FALSE);
}
