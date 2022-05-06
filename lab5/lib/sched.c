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
}

void schedule() {
    if (rq_len == 1) return;  // scheduling is not necessary

    // get next task
    task_struct_t* next = next_task(get_current());
    // do {
    //     // simply choose the next task
    //     next = container_of(next->node.next, task_struct_t, node);

    //     // however, should never choose the main task or the current task
    // } while (&next->node == &rq || next == get_current());

    // printf("Choosen node: 0x%X 0x%X\n", next, &next->info);
    // printf("Current node: 0x%X 0x%X\n", get_current(), &get_current()->info);
    switch_to(get_current(), &next->info);
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

    disable_interrupt();

    task_struct_t* next = next_task(target);

    // delete target from run queue
    list_del(&target->node);
    rq_len -= 1;

    // add target to dead queue
    INIT_LIST_HEAD(&target->node);
    list_add_tail(&target->node, &dq);
    dq_len += 1;

    // if the current task is dead, switch to the next task
    if (target == get_current()) {
        enable_interrupt();
        switch_to(target, next);
    } else {
        enable_interrupt();
    }
}

task_struct_t* next_task(task_struct_t* curr) {
    task_struct_t* next = curr;
    do {
        // simply choose the next task
        next = container_of(next->node.next, task_struct_t, node);

        // however, should never choose the main task or the current task
    } while (&next->node == &rq || next == get_current());
    return next;
}

uint32_t create_kern_task(void (*func)(), void* arg) {
    task_struct_t* task = new_task();
    thread_info_t* info = &task->info;

    task->pid = 0;
    task->status = STOPPED;
    task->prio = 2;
    task->user_stack = 0;

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

    disable_interrupt();
    list_add_tail(&task->node, &rq);
    rq_len += 1;
    enable_interrupt();
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
    thread_release(get_current(), 0);
}

void show_rq() {
    task_struct_t* curr;
    list_for_each_entry(curr, &rq, node) {
        printf("0x%X -> ", curr->prio);
    }
    printf("\n");
}