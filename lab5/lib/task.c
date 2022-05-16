#include "task.h"

struct list_head* task_event_list;

void task_list_init() {
    task_event_list = kmalloc(sizeof(struct list_head));
    INIT_LIST_HEAD(task_event_list);
}

void run_task() {
    // TODO: if there is no task, maybe do something?
    if (list_empty(task_event_list)) {
        // printf("[+] task_event_list is empty" ENDL);
        return;
    }

    // trigger the first task
    task_event_t* t_event = container_of(task_event_list->next, task_event_t, node);
    ((void (*)())t_event->callback)();
    // ((void (*)())((task_event_t*)task_event_list->next)->callback)();

    // remove the first event
    disable_intr();  // critical section
    void* bk = task_event_list->next;
    list_del(task_event_list->next);
    kfree(bk);
    enable_intr();  // end of critical section

    // TODO: maybe handle the next task?
}

void add_task(void* callback, uint32_t priority) {
    //disable_interrupt();
    task_event_t* new_task_event = kmalloc(sizeof(task_event_t));
    //printf("[+] add_task -> 0x%X" ENDL, new_task_event);
    INIT_LIST_HEAD(&new_task_event->node);
    new_task_event->callback = callback;
    new_task_event->priority = priority;

    disable_intr();  // critical section
    task_event_t* curr;
    bool inserted = false;
    list_for_each_entry(curr, task_event_list, node) {
        if (new_task_event->priority < curr->priority) {
            list_add(&new_task_event->node, curr->node.prev);
            inserted = true;
        }
    }
    if (!inserted) list_add_tail(&new_task_event->node, task_event_list);
    // show_task_list();
    enable_intr();  // end of critical section
}

void show_task_list() {
    task_event_t* curr;
    bool inserted = false;
    printf("show_task_list()" ENDL);
    list_for_each_entry(curr, task_event_list, node) {
        printf("0x%lX -> ", curr->priority);
    }
    printf(ENDL);
}