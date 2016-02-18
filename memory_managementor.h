#ifndef MEMORY_MANAGEMENTOR_H
#define MEMORY_MANAGEMENTOR_H 1

#define MAX_PAGES 50
#define MAX_FRAMES 64
#define WORKING_SET_LIMIT 4
#define CLOCKS_FOR_DISK_ACCESS 100000

typedef unsigned int uint;
typedef int bool;
#define FALSE 0
#define TRUE 1

typedef struct {
	bool is_available;
	uint frame;
} page_info;

typedef struct {
	page_info pages[MAX_PAGES];
	uint LRU_pages[WORKING_SET_LIMIT];
	uint allocated_pages_number;
	pthread_mutex_t LRU_mutex;
	clock_t most_recent_page_time;
} page_table;

typedef struct _frame_list_node{
	uint frame;
	struct _frame_list_node* next;
} frame_list_node;

void init_m_managementor(void);
void m_managementor_get_page(uint, uint);
void insert_LRU(uint, uint);
void update_LRU(uint, uint);
void replace_LRU(uint, uint);

uint remove_free_frame(void);

void swap_out_process(uint);

void wait_clocks(clock_t);

#endif
