#include "task.h"

struct list_head* task_event_list;

void task_list_init() {
    task_event_list = kmalloc(sizeof(struct list_head));
    INIT_LIST_HEAD(task_event_list);
}

void run_task() {
    // TODO: if there is no task, maybe do something?
    if (list_empty(task_event_list)) {
        printf("[+] task_event_list is empty" ENDL);
        return;
    }

    // trigger the first task
    ((void (*)())((task_event_t*)task_event_list->next)->callback)();

    // remove the first event
    disable_interrupt();  // critical section
    list_rotate_left(task_event_list);
    void* ptr_bk = task_event_list->prev;  // !! backup the ptr
    list_del(task_event_list->prev);
    kfree(ptr_bk);
    enable_interrupt();  // end of critical section

    // TODO: maybe handle the next task?
}

void add_task(void* callback, uint32_t priority) {
    //disable_interrupt();
    task_event_t* new_task_event = kmalloc(sizeof(task_event_t));
    //printf("[+] add_task -> 0x%X" ENDL, new_task_event);
    INIT_LIST_HEAD(&new_task_event->node);
    new_task_event->callback = callback;
    new_task_event->priority = priority;

    disable_interrupt();  // critical section
    struct list_head* curr;
    bool inserted = false;
    list_for_each(curr, task_event_list) {
        if (new_task_event->priority < ((task_event_t*)curr)->priority) {
            list_add(&new_task_event->node, curr->prev);
            inserted = true;
        }
    }
    if (!inserted) list_add_tail(&new_task_event->node, task_event_list);
    //show_task_list();
    enable_interrupt();  // end of critical section
}

void show_task_list() {
    struct list_head* curr;
    bool inserted = false;
    printf("show_task_list()" ENDL);
    list_for_each(curr, task_event_list) {
        printf("0x%lX -> ", ((task_event_t*)curr)->priority);
    }
    printf(ENDL);
}