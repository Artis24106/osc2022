#ifndef __TASK_H__
#define __TASK_H__

#include "exception.h"
#include "list.h"
#include "malloc.h"
#include "string.h"

#define PRIORITY_NORMAL 0xdeadbeef          // normal irq interrupt
#define PRIORITY_UART_READ 0xdeadbeef - 1   // uart read irq interrupt
#define PRIORITY_UART_WRITE 0xdeadbeef - 2  // uart write irq interrupt
#define PRIORITY_TIMER 0xdeadc0de           // normal timer interrupt, just for debug
#define PRIORITY_PREEMPTION 0x1337          // high priority

typedef struct task_event {
    struct list_head node;
    void* callback;
    uint32_t priority;
} task_event_t;

void task_list_init();
void run_task();
void add_task(void* callback, uint32_t priority);
void show_task_list();

#endif