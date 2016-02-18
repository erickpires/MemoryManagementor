#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include "work_thread.h"
#include "memory_managementor.h"
#include "logger.h"


#include "work_thread.c"
#include "memory_managementor.c"
#include "logger.c"

// TODO: Create a log system and send all message to it. Avoid lost of
//       concurrency by writing data to a file

// TODO: Wait some random period of time between each thread creation

#define TIME_BETWEEN_THREADS 3

bool is_running = TRUE;

void finish_handler(int sig) {
	is_running = FALSE;
	if(!stop_threads()) {
		fprintf(stderr, "Could not stop threads safely\n");
		exit(1);
	}
}

int main(int argc, char** argv){

	signal(SIGINT, finish_handler);
	signal(SIGQUIT, finish_handler);

	init_logger();
	init_m_managementor();

	for(int i = 0; i < MAX_THREADS && is_running; i++) {
		create_thread();
		sleep(TIME_BETWEEN_THREADS);
	}

	join_threads();

    return 0;
}
