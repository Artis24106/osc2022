#ifndef __SVC_H__
#define __SVC_H__

#include "cpio.h"
#include "mbox.h"
#include "sched.h"
#include "stdint.h"
#include "string.h"

int32_t sys_getpid();
int64_t sys_uartread(char buf[], int64_t size);
int64_t sys_uartwrite(const char buf[], int64_t size);
int32_t sys_exec(trap_frame_t* tf, const char* name, char* const argv[]);
int32_t sys_fork(trap_frame_t* tf);
void sys_exit(int32_t status);
int32_t sys_mbox_call(uint8_t ch, mail_t* mbox);
void sys_kill(int32_t pid);

#define SYS_NUM 8

extern void (*sys_tbl[SYS_NUM])(void*);

void sys_handler(trap_frame_t* tf);

#endif