#include "string.h"

int strcmp(const char *a, const char *b) {
    uint32_t i = 0;
    while (a[i] == b[i] && a[i] != '\0' && b[i] != '\0') i++;
    return a[i] - b[i];
}

int strncmp(const char *a, const char *b, uint32_t size) {
    uint32_t i = 0;
    while (a[i] == b[i] && i < size - 1) i++;
    return a[i] - b[i];
}

uint32_t strlen(const char *a) {
    for (uint32_t i = 0;; i++)
        if (a[i] == '\0') return i;
    return 0;
}

uint32_t hex_ascii_to_uint32(const char *str, uint32_t size) {
    uint32_t ret = 0;
    for (uint32_t i = 0; i < size; i++) {
        ret *= 16;
        if (str[i] >= '0' && str[i] <= '9') {
            ret += str[i] - '0';
        } else if (str[i] >= 'a' && str[i] <= 'f') {
            ret += str[i] - 'a' + 10;
        } else if (str[i] >= 'A' && str[i] <= 'F') {
            ret += str[i] - 'A' + 10;
        }
    }
    return ret;
}

uint32_t get_be_uint32(void *ptr) {
    uint8_t *bytes = (uint8_t *)ptr;
    return bytes[3] | bytes[2] << 8 | bytes[1] << 16 | bytes[0] << 24;
}

char *memcpy(void *dest, const void *src, uint64_t len) {
    uint32_t daif = get_intr();
    disable_intr();
    char *d = dest;
    char *s = src;
    while (len--) *d++ = *s++;
    set_intr(daif);
    return dest;
}

void memset(char *dest, char c, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) *dest++ = c;
}

char *strcpy(char *dest, char *src) {
    return memcpy(dest, src, strlen(src) + 1);
}

char *strchr(const char *str, int ch) {
    for (uint64_t i = 0; str[i] != '\0'; i++) {
        if (str[i] == ch) return str + i;
    }
    return;
}

uint32_t atoi(const char *nptr) {
    uint32_t t = 0;
    for (uint64_t i = 0; nptr[i] != '\0'; i++) {
        t *= 10;
        t += nptr[i] - '0';
    }
    return t;
}