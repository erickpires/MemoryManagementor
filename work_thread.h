#ifndef WORK_THREAD_H
#define WORK_THREAD_H 1

#define MAX_THREADS 20

#define SLEEP_TIME 3

typedef unsigned int uint;
typedef int bool;
#define FALSE 0
#define TRUE 1

typedef struct {
	uint t_id;
} thread_info;

void* thread_code(void*);
void create_thread(void);
void join_threads(void);
bool stop_threads(void);

#endif
