#include "sched.h"

task_struct_t* main_task;

struct list_head rq = LIST_HEAD_INIT(rq);  // run queue
struct list_head wq = LIST_HEAD_INIT(wq);  // wait queue
struct list_head dq = LIST_HEAD_INIT(dq);  // dead queue

uint32_t rq_len = 0,
         wq_len = 0,
         dq_len = 0;

void idle() {
    while (true) {
        kill_zombies();
        schedule();
    }
}

void kill_zombies() {
    disable_intr();

    // free all tasks in the dead queue
    task_struct_t *curr, *temp;
    list_for_each_entry_safe(curr, temp, &dq, node) {  // `safe` is needed when deleting nodes
        list_del(&curr->node);
        if (curr->user_stack) kfree(curr->user_stack);
        kfree(curr->kernel_stack);
        kfree(&curr->node);

        dq_len -= 1;
    }

    enable_intr();
}

void schedule() {
    if (rq_len == 1) return;  // scheduling is not necessary

    // get next task
    task_struct_t* next = next_task(current);
    switch_to(current, &next->info);
}

void main_thread_init() {
    main_task = new_task();
    main_task->pid = 0;
    main_task->status = RUNNING;
    main_task->prio = 1;
    main_task->user_stack = 0;
    main_task->kernel_stack = 0x100000;  // TODO: somewhere else
    INIT_LIST_HEAD(&main_task->node);

    rq_len += 1;
    list_add(&main_task->node, rq.next);
    write_sysreg(tpidr_el1, main_task);
    // update_timer();
    schedule();
}

void thread_create(void* start) {
}

void thread_release(task_struct_t* target, int16_t ec) {
    // never kill the main task
    if (target == main_task) return;

    target->exit_code = ec;
    target->status = DEAD;

    disable_intr();

    task_struct_t* next = next_task(target);

    // delete target from run queue
    list_del(&target->node);
    rq_len -= 1;

    // add target to dead queue
    INIT_LIST_HEAD(&target->node);
    list_add_tail(&target->node, &dq);
    dq_len += 1;

    // if the current task is dead, switch to the next task
    if (target == current) {
        enable_intr();
        switch_to(target, next);
    } else {
        enable_intr();
    }
}

task_struct_t* next_task(task_struct_t* curr) {
    task_struct_t* next = curr;
    do {
        // simply choose the next task
        next = container_of(next->node.next, task_struct_t, node);

        // however, should never choose the main task or the current task
    } while (&next->node == &rq || next == current);
    return next;
}

uint32_t create_kern_task(void (*func)(), void* arg) {
    task_struct_t* task = new_task();
    thread_info_t* info = &task->info;

    task->pid = 0;
    task->status = STOPPED;
    task->prio = 2;
    task->user_stack = NULL;

    // store function addr and args
    info->x19 = func;
    info->x20 = arg;

    // malloc kernel stack
    info->sp = kmalloc(THREAD_STACK_SIZE);
    task->kernel_stack = info->sp;
    info->sp += THREAD_STACK_SIZE - 8;  // TODO: not sure about the `-8` part
    info->fp = info->sp;

    // the trampoline will swap some registers, then call the function
    //  -> swap(x19, x0), swap(x20, x1)
    //  -> func(argv)
    info->lr = _thread_trampoline;  // link register -> hold the return address

    disable_intr();
    list_add_tail(&task->node, &rq);
    rq_len += 1;
    enable_intr();
}

task_struct_t* new_task() {
    task_struct_t* task = kmalloc(sizeof(task_struct_t));
    memset(task, 0, sizeof(task_struct_t));
    task->time = 1;
    INIT_LIST_HEAD(&task->node);
    return task;
}

void thread_trampoline(void (*func)(), void* argv) {
    func(argv);
    thread_release(current, EX_OK);
}

void show_q() {
    task_struct_t* curr;
    printf("rq: ");
    list_for_each_entry(curr, &rq, node) {
        printf("0x%X -> ", curr->prio);
    }
    printf("\n");

    printf("wq: ");
    list_for_each_entry(curr, &wq, node) {
        printf("0x%X -> ", curr->prio);
    }
    printf("\n");

    printf("dq: ");
    list_for_each_entry(curr, &dq, node) {
        printf("0x%X -> ", curr->prio);
    }
    printf("\n\n");
}