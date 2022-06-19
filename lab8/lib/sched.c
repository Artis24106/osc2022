#include "sched.h"

task_struct_t* main_task;

struct list_head rq = LIST_HEAD_INIT(rq);  // run queue
struct list_head wq = LIST_HEAD_INIT(wq);  // wait queue
struct list_head dq = LIST_HEAD_INIT(dq);  // dead queue

uint32_t rq_len = 0,
         wq_len = 0,
         dq_len = 0;

uint64_t current_pid = 1;

void set_current(uint64_t val) {
    write_sysreg(tpidr_el1, val);
}

void idle() {
    printf("idle()" ENDL);
    while (true) {
        kill_zombies();
        schedule();
    }
}

void kill_zombies() {
    if (dq_len <= 0) return;

    // disable_intr();

    // free all tasks in the dead queue
    task_struct_t *curr, *temp;
    list_for_each_entry_safe(curr, temp, &dq, node) {  // `safe` is needed when deleting nodes
        list_del(&curr->node);
        if (curr->user_stack) kfree(curr->user_stack);
        kfree(curr->kernel_stack);
        kfree(&curr->node);

        dq_len -= 1;
    }

    // enable_intr();
}

void schedule() {
    if (rq_len <= 1) return;  // scheduling is not necessary

    // if (!check_lock()) {
    //     return;
    // };
    // lock();
    // get next task
    task_struct_t* next = next_task(current);

    // set_intr(daif);
    update_timer();
    // printf("sch switch" ENDL);
    task_struct_t* curr = current;
    // printf("curr: 0x%X (0x%X)" ENDL, &curr->node, curr);
    // show_q();
    // for (uint32_t i = 0; i < 0x1000; i++) asm("nop");
    switch_to(current, &next->info);
}

void try_schedule() {  // schedule callback for timer
    // printf("try_schedule()" ENDL);
    // printf("rq_len -> %d" ENDL, rq_len);
    if (current != NULL && current->time <= 0) {
        // printf("----------------------------try" ENDL);
        schedule();
        current->time = 1;  // TODO: why 1
    }

    update_timer();
}

void try_signal_handler(trap_frame_t* tf) {
    task_struct_t* curr = current;

    // try getting the first signal
    signal_t* signal = list_first_entry_or_null(&curr->signal->node, signal_t, node);
    if (!signal) return;

    sigaction_t* sigaction = &curr->sighand->sigactions[signal->signum];
    printf("try_signal_handler -> 0x%X" ENDL, sigaction->sa_handler);
    if (sigaction->is_kernel_hand) {
        printf("is_kernel_hand" ENDL);
        sigaction->sa_handler(signal->signum);
    } else {
        printf("not is_kernel_hand" ENDL);
        void* user_sp = tf->sp_el0 - align(sizeof(trap_frame_t), 0x10);
        memcpy(user_sp, tf, sizeof(trap_frame_t));

        tf->sp_el0 = user_sp;  // save user sp, which will be restored when `sys_sigreturn`

        tf->x0 = signal->signum;              // set pararmeter
        tf->elr_el1 = sigaction->sa_handler;  // user pc will return to handler

        tf->x30 = sigreturn;  // set lr to sigreturn
    }
    list_del(&signal->node);
    kfree(signal);
}

void main_thread_init() {
    printf("main_thread_init()" ENDL);
    main_task = new_task();
    main_task->pid = 0;
    main_task->status = RUNNING;
    main_task->prio = 1;
    main_task->user_stack = 0;
    main_task->kernel_stack = STARTUP_HEAP_END;  // TODO: somewhere else
    INIT_LIST_HEAD(&main_task->node);

    rq_len += 1;
    // list_add(&main_task->node, rq.next);  // add main task into run queue
    list_add_tail(&main_task->node, &rq);

    write_sysreg(tpidr_el1, main_task);

    // uint64_t x0 = read_sysreg(cntfrq_el0);
    // add_timer(sched_callback, NULL, x0 >> 5, false);
}

void thread_create(void* start) {
}

void thread_release(task_struct_t* target, int16_t ec) {
    // never kill the main task
    if (target == main_task) return;

    target->exit_code = ec;
    target->status = DEAD;
    uint32_t daif = get_intr();
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
        // update_timer();
        set_intr(daif);
        // printf("dis switch_to" ENDL);
        switch_to(target, next);
    } else {
        set_intr(daif);
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

task_struct_t* get_task_by_pid(uint32_t pid) {
    task_struct_t* curr;

    // run queue
    list_for_each_entry(curr, &rq, node) {
        if (curr->pid != pid) continue;
        goto get_task_by_pid_end;
    }

    // wait queue
    list_for_each_entry(curr, &wq, node) {
        if (curr->pid != pid) continue;
        goto get_task_by_pid_end;
    }

get_task_by_pid_end:
    return curr;
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
    info->sp += THREAD_STACK_SIZE - 0x10;  // XXX: stack alignment???
    info->fp = info->sp;

    // the trampoline will swap some registers, then call the function
    //  -> swap(x19, x0), swap(x20, x1)
    //  -> func(argv)
    info->lr = _kern_thread_trampoline;  // link register -> hold the return address

    // disable_intr();
    list_add_tail(&task->node, &rq);
    rq_len += 1;
    // enable_intr();
}

uint32_t create_user_task(char* path_name) {
    task_struct_t* task = new_task();
    thread_info_t* info = &task->info;
    file_t f;
    int ret;

    // open the file
    ret = vfs_open(path_name, 0, &f);
    if (ret < 0) return ret;

    // get file size
    int file_size = f.vnode->v_ops->get_size(f.vnode);
    if (file_size < 0) return -1;

    char* file_buf = kmalloc(file_size);

    // read
    ret = vfs_read(&f, file_buf, file_size);
    if (ret < 0) return ret;

    // close
    vfs_close(&f);

    // task->pid = current->pid + 1;  // TODO: maybe wrong
    task->pid = current_pid++;
    task->status = STOPPED;
    task->prio = 2;

    // store entry point
    info->x19 = file_buf;

    // malloc user stack
    info->x20 = kmalloc(THREAD_STACK_SIZE);
    task->user_stack = info->x20;
    info->x20 += THREAD_STACK_SIZE - 0x10;

    // malloc kernel stack
    info->sp = kmalloc(THREAD_STACK_SIZE);
    task->kernel_stack = info->sp;
    info->sp += THREAD_STACK_SIZE - 0x10;  // TODO: not sure about the `-8` part
    info->fp = info->sp;

    // the trampoline will change from el1 to el0
    info->lr = _user_thread_trampoline;  // link register -> hold the return address

    // disable_intr();
    list_add_tail(&task->node, &rq);
    rq_len += 1;
    // enable_intr();
}

uint32_t _fork(trap_frame_t* tf) {
    task_struct_t* task = new_task();
    task_struct_t* curr_task = current;

    task->pid = current_pid++;
    task->prio = curr_task->prio;  // inherit the old priority
    task->status = RUNNING;

    // copy user, kernel stack
    // task->user_stack = kmalloc(THREAD_STACK_SIZE);
    // task->kernel_stack = kmalloc(THREAD_STACK_SIZE);
    task->user_stack = kmalloc(THREAD_STACK_SIZE);
    task->kernel_stack = kmalloc(THREAD_STACK_SIZE);
    // printf("child user_stk: 0x%X, kern_stk: 0x%X" ENDL, task->user_stack, task->kernel_stack);
    memcpy(task->user_stack, curr_task->user_stack, THREAD_STACK_SIZE);
    memcpy(task->kernel_stack, curr_task->kernel_stack, THREAD_STACK_SIZE);

    // calculate the stack offset
    int64_t usp_off = task->user_stack - curr_task->user_stack;  // offset to user stack
    int64_t ksp_off = task->kernel_stack - curr_task->kernel_stack;

    // set the stack pointer to right place
    task->info = curr_task->info;
    thread_info_t* info = &task->info;
    info->fp += ksp_off;
    info->sp += ksp_off;
    info->lr = _fork_child_trampoline;  // this is for child

    // store some info for _fork_child_trampoline
    info->x19 = tf->x30;               // elr_el1
    info->x20 = tf->sp_el0 + usp_off;  // store new sp_el0 in x20

    // copy signal handler
    for (uint32_t i = 1; i < SIGRTMAX; i++) {
        task->sighand->sigactions[i].is_kernel_hand = curr_task->sighand->sigactions[i].is_kernel_hand;
        task->sighand->sigactions[i].sa_handler = curr_task->sighand->sigactions[i].sa_handler;
    }

    // add to run queue
    list_add_tail(&task->node, &rq);
    rq_len += 1;
    // enable_intr();

    // set return value
    tf->x0 = task->pid;
    return task->pid;
}

task_struct_t* new_task() {
    task_struct_t* task = kmalloc(sizeof(task_struct_t));
    memset(task, 0, sizeof(task_struct_t));
    task->time = 1;
    INIT_LIST_HEAD(&task->node);

    // signal
    task->signal = new_signal();
    task->sighand = new_sighand();

    // file
    task->max_fd = MAX_FD;
    task->work_dir = rootfs->root;
    for (int i = 0; i < MAX_FD; i++) task->fds[i].vnode = NULL;
    vfs_open("/dev/uart", 0, &task->fds[0]);  // stdin
    vfs_open("/dev/uart", 0, &task->fds[1]);  // stdout
    vfs_open("/dev/uart", 0, &task->fds[2]);  // stderr

    return task;
}

void thread_trampoline(void (*func)(), void* argv) {
    func(argv);
    thread_release(current, EX_OK);
}

uint64_t show_q_cnt = 0;
void show_q() {
    show_q_cnt++;
    task_struct_t* curr;
    printf("show_q_cnt: %d" ENDL, show_q_cnt);
    printf("rq_len: %d" ENDL, rq_len);
    printf("rq: ");
    uint64_t cnt = 0;
    list_for_each_entry(curr, &rq, node) {
        // printf("0x%X -> ", curr->pid);
        printf("0x%X -> ", &curr->node);
        cnt += 1;
        if (cnt > rq_len) {
            // return;
            break;
        }
    }
    printf("\n");

    // printf("wq: ");
    // list_for_each_entry(curr, &wq, node) {
    //     printf("0x%X -> ", curr->pid);
    // }
    // printf("\n");

    // printf("dq: ");
    // list_for_each_entry(curr, &dq, node) {
    //     printf("0x%X -> ", curr->pid);
    // }
    // printf("\n\n");
    printf(ENDL);
}

uint32_t update_cnt = 0;
void update_timer() {
    // printf("update_timer(0x%X)\r\n", update_cnt++);
#ifndef DISABLE_CONTEXT_SWITCH
    set_timeout_rel(5);
    enable_timer();
#endif
}

void show_task(task_struct_t* task) {
    printf("show_task(0x%X)" ENDL, task);
    printf("\t-> info = 0x%X" ENDL, task->info);
    printf("\t-> pid = 0x%X" ENDL, task->pid);
    printf("\t-> status = 0x%X" ENDL, task->status);
    printf("\t-> exit_code = 0x%X" ENDL, task->exit_code);
    printf("\t-> prio = 0x%X" ENDL, task->prio);
    printf("\t-> time = 0x%X" ENDL, task->time);
    printf("\t-> user_stack = 0x%X" ENDL, task->user_stack);
    printf("\t-> kernel_stack = 0x%X" ENDL, task->kernel_stack);
    printf(ENDL);
}

void show_tf(trap_frame_t* tf) {
    printf(ENDL);
    printf("x0: 0x%X" ENDL, tf->x0);
    printf("x1: 0x%X" ENDL, tf->x1);
    printf("x2: 0x%X" ENDL, tf->x2);
    printf("x3: 0x%X" ENDL, tf->x3);
    printf("x4: 0x%X" ENDL, tf->x4);
    printf("x5: 0x%X" ENDL, tf->x5);
    printf("x6: 0x%X" ENDL, tf->x6);
    printf("x7: 0x%X" ENDL, tf->x7);
    printf("x8: 0x%X" ENDL, tf->x8);
    printf("x9: 0x%X" ENDL, tf->x9);
    printf("x10: 0x%X" ENDL, tf->x10);
    printf("x11: 0x%X" ENDL, tf->x11);
    printf("x12: 0x%X" ENDL, tf->x12);
    printf("x13: 0x%X" ENDL, tf->x13);
    printf("x14: 0x%X" ENDL, tf->x14);
    printf("x15: 0x%X" ENDL, tf->x15);
    printf("x16: 0x%X" ENDL, tf->x16);
    printf("x17: 0x%X" ENDL, tf->x17);
    printf("x18: 0x%X" ENDL, tf->x18);
    printf("x19: 0x%X" ENDL, tf->x19);
    printf("x20: 0x%X" ENDL, tf->x20);
    printf("x21: 0x%X" ENDL, tf->x21);
    printf("x22: 0x%X" ENDL, tf->x22);
    printf("x23: 0x%X" ENDL, tf->x23);
    printf("x24: 0x%X" ENDL, tf->x24);
    printf("x25: 0x%X" ENDL, tf->x25);
    printf("x26: 0x%X" ENDL, tf->x26);
    printf("x27: 0x%X" ENDL, tf->x27);
    printf("x28: 0x%X" ENDL, tf->x28);
    printf("x29: 0x%X" ENDL, tf->x29);
    printf("x30: 0x%X" ENDL, tf->x30);
}