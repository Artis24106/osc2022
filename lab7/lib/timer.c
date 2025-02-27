#include "timer.h"

struct list_head* timer_event_list;

void enable_timer() {
    // printf("enable_timer()" ENDL);
    *(uint32_t*)CORE0_TIMER_IRQ_CTRL = 2;  // enable rip3 timer interrupt
}

void disable_timer() {
    // printf("disable_timer()" ENDL);
    *(uint32_t*)CORE0_TIMER_IRQ_CTRL = 0;  // disable rip3 timer interrupt
}

void timer_handler() {
    printf("timer_handler()" ENDL);
    current->time--;

    // ok, do nothing
}

void timer_callback() {
    // if there is no timer event, set a huge expire time
    if (list_empty(timer_event_list)) {
        // printf("[+] timer_event_list is empty" ENDL);
        set_timeout_rel(65535);
        goto timer_callback_end;
    }

    // trigger the first callback
    timer_event_t* target = container_of(timer_event_list->next, timer_event_t, node);
    target->callback(target->args);

    // disable_timer();
    // remove the first event
    void* bk = target;
    list_del(&target->node);
    kfree(bk);

    // if there is next event, set next timeout
    if (list_empty(timer_event_list)) {
        // set_timeout_rel(65535);
    } else {
        target = container_of(timer_event_list->next, timer_event_t, node);
        // set_timeout_abs(target->tval);
    }

timer_callback_end:
    show_timer_list();
    // [ Lab3 - AD2 ] 5. unmasks the interrupt line to get the next interrupt at the end of the task.
    enable_timer();
}

void timer_list_init() {
    timer_event_list = kmalloc(sizeof(struct list_head));
    INIT_LIST_HEAD(timer_event_list);
}

uint64_t get_absolute_time(uint64_t offset) {
    uint64_t cntpct_el0, cntfrq_el0;
    get_reg(cntpct_el0, cntpct_el0);
    get_reg(cntfrq_el0, cntfrq_el0);
    return cntpct_el0 + cntfrq_el0 * offset;
}

void add_timer(void* callback, char* args, uint64_t timeout, bool is_abs) {
    // disable_intr();

    timer_event_t* t_event = kmalloc(sizeof(timer_event_t));
    INIT_LIST_HEAD(&t_event->node);
    t_event->args = kmalloc(strlen(args) + 1);  // TODO: malloc bug here QQ, may write the run queue
    strcpy(t_event->args, args);
    t_event->callback = callback;
    if (is_abs) {
        t_event->tval = get_absolute_time(timeout);
    } else {
        t_event->tval = get_absolute_time(0) + timeout;
    }

    // if the list is empty, set first event interrupt
    // if (list_empty(timer_event_list)) set_timeout_abs(t_event->tval);

    // insert node
    timer_event_t* curr;
    bool inserted = false, is_first_node = true;
    list_for_each_entry(curr, timer_event_list, node) {
        if (t_event->tval < curr->tval) {
            list_add(&t_event->node, curr->node.prev);
            inserted = true;
            break;
        }
        is_first_node = false;
    }
    if (!inserted) list_add_tail(&t_event->node, timer_event_list);
    // show_timer_list();
    // if the first element is updated, should renew the timeout
    // enable_timer();
    // if (is_first_node) set_timeout_abs(t_event->tval);

    // enable_intr();
}

void show_timer_list() {
    timer_event_t* curr;
    bool inserted = false;
    printf("[DEBUG] show_timer_list()" ENDL);
    list_for_each_entry(curr, timer_event_list, node) {
        printf("0x%X -> ", curr->tval);
    }
    printf(ENDL);
}

void sleep(uint64_t timeout) {
    add_timer(NULL, NULL, 2, true);
}

void show_msg_callback(char* args) {
    printf("[+] show_msg_callback() -> %s" ENDL, args);
}

void show_time_callback(char* args) {
    uint64_t cntpct_el0, cntfrq_el0;
    get_reg(cntpct_el0, cntpct_el0);
    get_reg(cntfrq_el0, cntfrq_el0);
    printf("[+] show_time_callback() -> %02ds" ENDL, cntpct_el0 / cntfrq_el0);

    add_timer(show_time_callback, args, 2, true);
}

void sched_callback() {
    printf("sched_callback()" ENDL);
    try_schedule();
    uint64_t x0 = read_sysreg(cntfrq_el0);
    add_timer(sched_callback, NULL, x0 >> 5, false);
    enable_timer();
}

void set_timeout_rel(uint64_t timeout) {  // relative -> cntp_tval_el0
    uint64_t x0 = read_sysreg(cntfrq_el0);
    write_sysreg(cntp_tval_el0, x0 >> timeout);
    // write_sysreg(cntp_tval_el0, 0x10000);
}

void set_timeout_abs(uint64_t timeout) {  // absoulute -> cntp_cval_el0
    write_sysreg(cntp_cval_el0, timeout);
}
