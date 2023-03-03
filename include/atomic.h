#pragma once

#include <include/types.h>

typedef struct {
    int64_t counter;
} __attribute__ ((aligned (8))) atomic_t;

#define ATOMIC_INIT(i) { (i) }

static inline void atomic_add(int64_t i, atomic_t *v) {
    uint32_t tmp;
    int64_t result;

    __asm__ volatile("// atomic_add\n"
        "1: ldaxr %x0, %2\n"
        "add %x0, %x0, %x3\n"
        "stlxr %w1, %x0, %2\n"
        "cbnz %w1, 1b"
        : "=&r" (result), "=&r" (tmp), "+Q" (v->counter)
        : "Ir" (i));
}

static inline void atomic_set(int64_t i, atomic_t *v) {
    WRITE_ONCE(v->counter, i);
}

static inline int64_t atomic_read(atomic_t *v) {
    return READ_ONCE(v->counter);
}

static inline bool atomic_compare_and_swap(atomic_t *v, uint64_t old, uint64_t new) {
    uint32_t res;
    uint64_t oldval;
    __asm__ volatile("// atomic_cas\n"
        "ldaxr %x1, %2\n"
        "mov %w0, 1\n"
        "cmp %x1, %x3\n"
        "bne 1f\n"
        "stlxr %w0, %x4, %2\n"
        "1:\n"
        : "=&r" (res), "=&r" (oldval), "+Q" (v->counter)
        : "Ir" (old), "Ir" (new));

    return !(bool)res;
}
