#ifndef __STRING_H__
#define __STRING_H__

#include "stdint.h"

#define ENDL "\r\n"

int strcmp(const char *a, const char *b) {
    uint32_t i = 0;
    while (a[i] == b[i] && a[i] != '\0' && b[i] != '\0') i++;
    return a[i] - b[i];
}

uint32_t strlen(const char *a) {
    for (uint32_t i = 0;; i++)
        if (a[i] == '\0') return i;
    return 0;
}

void hex(char *buf, uint32_t t) {
    for (uint32_t i = 0; i < 8; i += 2) {
        uint8_t msb = (t & 0xf0) >> 4,
                lsb = t & 0xf;
        buf[i] = (msb < 10) ? msb + '0' : (msb - 10) + 'a';
        buf[i + 1] = (lsb < 10) ? lsb + '0' : (lsb - 10) + 'a';
        t >>= 8;
    }
    buf[8] = '\0';
}

#endif