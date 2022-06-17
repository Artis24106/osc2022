#ifndef __SVC_H__
#define __SVC_H__

#include "cpio.h"
#include "fs/vfs.h"
#include "mbox.h"
#include "sched.h"
#include "stdint.h"
#include "string.h"

int32_t sys_getpid();  // 0
int64_t sys_uartread(char buf[], int64_t size);
int64_t sys_uartwrite(const char buf[], int64_t size);
int32_t sys_exec(trap_frame_t* tf, const char* name, char* const argv[]);
int32_t sys_fork(trap_frame_t* tf);  // 4
void sys_exit(int32_t status);
int32_t sys_mbox_call(uint8_t ch, mail_t* mbox);
void sys_kill(int32_t pid);
static void sys_signal(int32_t signal, void (*handler)());  // 8
void sys_sigkill(int32_t pid, int signal);
void sys_sigreturn(trap_frame_t* tf);
int sys_open(trap_frame_t* tf, const char* pathname, int flags);
int sys_close(trap_frame_t* tf, int fd);  // 12
int sys_write(trap_frame_t* tf, int fd, const void* buf, unsigned long count);
int sys_read(trap_frame_t* tf, int fd, void* buf, unsigned long count);
int sys_mkdir(trap_frame_t* tf, const char* pathname, unsigned mode);
int sys_mount(trap_frame_t* tf, const char* src, const char* target, const char* filesystem, unsigned long flags, const void* data);  // 16
int sys_chdir(trap_frame_t* tf, const char* path);
long sys_lseek64(trap_frame_t* tf, int fd, long offset, int whence);
int sys_ioctl(trap_frame_t* tf, int fd, unsigned long request, ...);

#define SYS_NUM (20)  // 3 for advance

extern void (*sys_tbl[SYS_NUM])(void*);
extern uint64_t current_pid;

void sys_handler(trap_frame_t* tf);
void sigkill_handler(int32_t pid);

#endif