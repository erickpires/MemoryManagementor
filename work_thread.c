#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "memory_managementor.h"

#define MAX_PAGES 50
#define MAX_THREADS 30

#define SLEEP_TIME 3

void* thread_code(void* _t_info) {
	thread_info* t_info = (thread_info*) _t_info;

	while(TRUE) {
		int random_page = rand() % MAX_PAGES;
		m_managementor_get_page(t_info->t_id, random_page);
		sleep(SLEEP_TIME);
	}
}

static pthread_t threads[MAX_THREADS];
static thread_info t_infos[MAX_THREADS];

void create_thread() {
	static uint current_thread_id = 0;

	thread_info* t_info = t_infos + current_thread_id;
	t_info->t_id = current_thread_id;
	pthread_create(threads + current_thread_id, NULL, thread_code, t_info);

	current_thread_id++;
}
