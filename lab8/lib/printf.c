#include "printf.h"

#include <stdbool.h>
#include <stdint.h>

/**
 * printf (from https://github.com/bztsrc/raspi3-tutorial/tree/master/12_printf)
   initial printf from github dont use any va_end, and it is also can run. (in assembly there is nothing compiled from __builtin_va_end)
 */
int printf(char *fmt, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    char buf[MAX_PRINT_BUF_SIZE];
    // we don't have memory allocation yet, so we
    // simply place our string after our code
    char *s = (char *)buf;
    // use sprintf to format our string
    int count = vsprintf(s, fmt, args);
    // print out as usual
    while (*s) {
        uart_write(*s++);
    }
    __builtin_va_end(args);
    return count;
}

int async_printf(char *fmt, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    char buf[MAX_PRINT_BUF_SIZE];
    // we don't have memory allocation yet, so we
    // simply place our string after our code
    char *s = (char *)buf;
    // use sprintf to format our string
    int count = vsprintf(s, fmt, args);
    // print out as usual
    while (*s) {
        async_uart_write(*s++);
    }
    __builtin_va_end(args);
    return count;
}

unsigned int vsprintf(char *dst, char *fmt, __builtin_va_list args) {
    long int arg;
    int len, sign, i;
    char *p, *orig = dst, tmpstr[19];

    // failsafes
    if (dst == (void *)0 || fmt == (void *)0) {
        return 0;
    }

    // main loop
    arg = 0;
    while (*fmt) {
        if (dst - orig > MAX_PRINT_BUF_SIZE - 0x10) {
            uart_write_string("Error!!! format string too long!!!!");
            return -1;
        }
        // argument access
        if (*fmt == '%') {
            fmt++;
            // literal %
            if (*fmt == '%') {
                goto put;
            }
            len = 0;
            // size modifier
            while (*fmt >= '0' && *fmt <= '9') {
                len *= 10;
                len += *fmt - '0';
                fmt++;
            }
            // skip long modifier
            if (*fmt == 'l') {
                fmt++;
            }
            // character
            if (*fmt == 'c') {
                arg = __builtin_va_arg(args, int);
                *dst++ = (char)arg;
                fmt++;
                continue;
            } else
                // decimal number
                if (*fmt == 'd') {
                arg = __builtin_va_arg(args, int);
                // check input
                sign = 0;
                if ((int)arg < 0) {
                    arg *= -1;
                    sign++;
                }
                if (arg > 99999999999999999L) {
                    arg = 99999999999999999L;
                }
                // convert to string
                i = 18;
                tmpstr[i] = 0;
                do {
                    tmpstr[--i] = '0' + (arg % 10);
                    arg /= 10;
                } while (arg != 0 && i > 0);
                if (sign) {
                    tmpstr[--i] = '-';
                }
                // padding, only space
                if (len > 0 && len < 18) {
                    while (i > 18 - len) {
                        tmpstr[--i] = ' ';
                    }
                }
                p = &tmpstr[i];
                goto copystring;
            } else
                // hex number
                if (*fmt == 'X') {
                arg = __builtin_va_arg(args, long int);
                // convert to string
                i = 16;
                tmpstr[i] = 0;
                do {
                    char n = arg & 0xf;
                    // 0-9 => '0'-'9', 10-15 => 'A'-'F'
                    tmpstr[--i] = n + (n > 9 ? 0x37 : 0x30);
                    arg >>= 4;
                } while (arg != 0 && i > 0);
                // padding, only leading zeros
                if (len > 0 && len <= 16) {
                    while (i > 16 - len) {
                        tmpstr[--i] = '0';
                    }
                }
                p = &tmpstr[i];
                goto copystring;
            } else
                // string
                if (*fmt == 's') {
                p = __builtin_va_arg(args, char *);
            copystring:
                if (p == (void *)0) {
                    p = "(null)";
                }
                while (*p) {
                    *dst++ = *p++;
                }
            }
        } else {
        put:
            *dst++ = *fmt;
        }
        fmt++;
    }
    *dst = 0;
    // number of bytes written
    return dst - orig;
}

/**
 * Variable length arguments
 */
unsigned int sprintf(char *dst, char *fmt, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    unsigned int r = vsprintf(dst, fmt, args);
    __builtin_va_end(args);
    return r;
}