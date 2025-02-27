// Ref: https://github.com/torvalds/linux/blob/master/tools/include/linux/types.h

/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _TOOLS_LINUX_TYPES_H_
#define _TOOLS_LINUX_TYPES_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef __SANE_USERSPACE_TYPES__
#define __SANE_USERSPACE_TYPES__ /* For PPC64, to get LL64 types */
#endif

struct page;
struct kmem_cache;

typedef enum {
    GFP_KERNEL,
    GFP_ATOMIC,
    __GFP_HIGHMEM,
    __GFP_HIGH
} gfp_t;

/*
 * We define u64 as uint64_t for every architecture
 * so that we can print it with "%"PRIx64 without getting warnings.
 *
 * typedef __u64 u64;
 * typedef __s64 s64;
 */
typedef uint64_t u64;
typedef int64_t s64;

typedef uint32_t u32;
typedef int32_t s32;

typedef uint16_t u16;
typedef int16_t s16;

typedef uint8_t u8;
typedef int8_t s8;

#ifdef __CHECKER__
#define __bitwise __attribute__((bitwise))
#else
#define __bitwise
#endif

#define __force
#define __user
#define __must_check
#define __cold

typedef uint16_t __bitwise __le16;
typedef uint16_t __bitwise __be16;
typedef uint32_t __bitwise __le32;
typedef uint32_t __bitwise __be32;
typedef uint64_t __bitwise __le64;
typedef uint64_t __bitwise __be64;

typedef uint16_t __bitwise __sum16;
typedef uint32_t __bitwise __wsum;

typedef struct {
    int counter;
} atomic_t;

#ifndef __aligned_u64
#define __aligned_u64 uint64_t __attribute__((aligned(8)))
#endif

typedef struct list_head {
    struct list_head *next, *prev;
} list_head_t;

struct hlist_head {
    struct hlist_node *first;
};

struct hlist_node {
    struct hlist_node *next, **pprev;
};

typedef uint64_t addr_t;

#endif /* _TOOLS_LINUX_TYPES_H_ */