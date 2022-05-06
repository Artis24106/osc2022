#ifndef __STRING_H__
#define __STRING_H__

#include <stdint.h>

#define ENDL "\r\n"

int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, uint32_t size);
uint32_t strlen(const char *a);
uint32_t hex_ascii_to_uint32(const char *str, uint32_t size);
uint32_t get_be_uint32(void *ptr);
char *memcpy(void *dest, const void *src, uint64_t len);
void memset(char *dest, char c, uint32_t size);
char *strcpy(char *dest, char *src);
char *strchr(const char *str, int ch);
uint32_t atoi(const char *nptr);

// TODO: move to other .h file
#define read_sysreg(reg) ({          \
    uint64_t _val;                   \
    __asm__ volatile("mrs x0, " #reg \
                     : "=r"(_val));  \
                    _val; })
#define write_sysreg(reg, _val) ({ __asm__ volatile("msr " #reg ", %0" ::"rZ"(_val)); })

#endif