#pragma once

// UP implementation
// TODO: lock variable for SMP spinlock
typedef struct {} arch_spinlock_t;

typedef struct spinlock {
    arch_spinlock_t lock;
} spinlock_t;
