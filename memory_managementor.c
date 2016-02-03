#include <stdio.h>
#include <stdlib.h>
#include "memory_managementor.h"

page_info m_managementor_get_page(uint id, uint page_number) {
	page_info result = {};

	printf("Execution Unity %d requested page: %d\n", id, page_number);

	return result;
}

