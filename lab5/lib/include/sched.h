#ifndef __SCHED_H__
#define __SCHED_H__

#include "list.h"
#include "malloc.h"
#include "string.h"

#define THREAD_STACK_SIZE 0x2000  // TODO: why

typedef struct thread_info {
    uint64_t x19;  // callee saved register
    uint64_t x20;
    uint64_t x21;
    uint64_t x22;
    uint64_t x23;
    uint64_t x24;
    uint64_t x25;
    uint64_t x26;
    uint64_t x27;
    uint64_t x28;
    uint64_t fp;  // = x29
    uint64_t lr;  // = x30
    uint64_t sp;
} thread_info_t;

typedef struct task_struct {
    // thread_info should be the first element
    thread_info_t info;

    struct list_head node;
    int32_t pid;

    int16_t status;
#define EMPTY (0)
#define STOPPED (1 << 0)
#define RUNNING (1 << 1)
#define WAITING (1 << 2)
#define DEAD (1 << 3)
    int16_t exit_code;
    uint32_t prio;
    uint32_t time;
    uint64_t user_stack;
    uint64_t kernel_stack;
    // signal
    // signal_ctx
    // signal_queue
} task_struct_t;

// run queue, wait queue, dead queue
extern struct list_head rq, wq, dq;

extern void switch_to(void* prev, void* next);
extern task_struct_t* get_current();

void idle();
void kill_zombies();
void schedule();
void main_thread_init();
void thread_create(void* start);
void thread_release(task_struct_t* target, int16_t ec);
task_struct_t* next_task(task_struct_t* curr);

uint32_t create_kern_task(void (*func)(), void* arg);
task_struct_t* new_task();

void _thread_trampoline();
void thread_trampoline(void (*func)(), void* argv);

void show_rq();

#endif