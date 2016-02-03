#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "work_thread.h"
#include "memory_managementor.h"

#include "work_thread.c"
#include "memory_managementor.c"

// TODO: Create a log system and send all message to it. Avoid lost of
//       concurrency by writing data to a file

// TODO: Wait some random period of time between each thread creation

void finish_handler(int sig) {
	if(!stop_threads()) {
		fprintf(stderr, "Could not stop threads safely\n");
		exit(1);
	}
}

int main(int argc, char** argv){

	signal(SIGINT, finish_handler);
	signal(SIGQUIT, finish_handler);

	for(int i = 0; i < MAX_THREADS; i++) {
		create_thread();
	}

	join_threads();

    return 0;
}
