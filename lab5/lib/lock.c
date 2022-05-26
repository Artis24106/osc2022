#include "lock.h"

#include "printf.h"

void lock_init() {
    lock_cnt = 0;
}

void lock() {
    // printf("[+] lock()" ENDL);
    lock_cnt++;
}

void unlock() {
    // printf("[+] unlock()" ENDL);
    lock_cnt--;
    if (lock_cnt < 0) {
        printf("[ERROR] unlock()" ENDL);
        while (1)
            ;
    }
}

bool check_lock() {
    return !(lock_cnt & 1);  // true, if not locked
}