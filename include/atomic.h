#pragma once

#include <include/types.h>
#include <include/spinlock_types.h>

// UP implementation
// TODO: enable data cache and use exclusive access assembly instructions
typedef struct {
    int64_t counter;
    spinlock_t lock;
} atomic_t;

#define ATOMIC_INIT(i) { (i), {} }

void atomic_add(int64_t, atomic_t *);
void atomic_set(int64_t, atomic_t *);
int64_t atomic_read(atomic_t *);
