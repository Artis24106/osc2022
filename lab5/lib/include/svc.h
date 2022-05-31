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
void sys_signal(int32_t signal, void (*handler)());
void sys_sigkill(int32_t pid, int signal);

#define SYS_NUM (8 + 2)  // 2 for advance

extern void (*sys_tbl[SYS_NUM])(void*);
extern uint64_t current_pid;

void sys_handler(trap_frame_t* tf);

#endif