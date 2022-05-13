#ifndef __EXEC_H__
#define __EXEC_H__

#include <stdint.h>

#include "malloc.h"
#include "page_alloc.h"

// #define USTACK_SIZE 0x10000  // 64KB
#define USTACK_SIZE 0x1000
// TODO: 64KB pages for translation table???

void exec(char* file_data, uint32_t data_size);
void _exec(char* file_data, uint32_t data_size);
void el1_to_el0(char* file_ptr, char* stack_ptr);

#endif