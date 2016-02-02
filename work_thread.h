#ifndef WORK_THREAD_H
#define WORK_THREAD_H 1

typedef unsigned int uint;

typedef struct {
	uint t_id
} thread_info;

void* thread_code(void*);
void create_thread(void);

#endif
