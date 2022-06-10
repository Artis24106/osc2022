#ifndef __LOCK_H__
#define __LOCK_H__

#include "types.h"

static int64_t lock_cnt;

void lock_init();
void lock();
void unlock();
bool check_lock();

#endif