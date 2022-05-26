#include "task.h"

list_head_t* task_event_list;

void task_list_init() {
    task_event_list = kmalloc(sizeof(struct list_head));
    INIT_LIST_HEAD(task_event_list);
}

void run_task() {
    // TODO: if there is no task, maybe do something?
    // printf("[+] task_event_list is empty" ENDL);
    // return;

    // Simply execute all tasks once irq_handler is triggered
    while (!list_empty(task_event_list)) {
        // show_task_list();
        // trigger the first task
        task_event_t* t_event = container_of(task_event_list->next, task_event_t, node);
        if (t_event->callback == 0) {  // XXX: check if callback is nullptr
            printf("CALLBACK(0x%X)" ENDL, t_event->callback);
        }
        ((void (*)())t_event->callback)();

        // remove the first event
        void* bk = t_event;
        list_del(&t_event->node);
        kfree(bk);

        // TODO: maybe handle the next task?
    }
}

void add_task(void* callback, uint32_t priority) {
    uint32_t daif = get_intr();
    disable_intr();  // critical section
    task_event_t* new_task_event = kmalloc(sizeof(task_event_t));
    // printf("[+] add_task -> 0x%X" ENDL, new_task_event);
    INIT_LIST_HEAD(&new_task_event->node);
    new_task_event->callback = callback;
    new_task_event->priority = priority;

    list_add_tail(&new_task_event->node, task_event_list);
    // show_task_list();
    // enable_intr();  // end of critical section
    set_intr(daif);
}

void show_task_list() {
    task_event_t* curr;
    bool inserted = false;
    printf("\x1b[38;5;1m[DEBUG] show_task_list():");
    list_for_each_entry(curr, task_event_list, node) {
        switch (curr->priority) {
            case PRIORITY_NORMAL:
                printf(" normal ->");
                break;
            case PRIORITY_TIMER:
                printf(" timer ->");
                break;
            default:
                printf(" other ->");
                break;
        }
    }
    printf("\x1b[0m" ENDL);
}