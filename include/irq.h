#pragma once

#include <include/types.h>

void irq_handler();
void peripheral_handler();

static inline void irq_disable() {
    __asm__ volatile("msr daifset, #2"
                     :
                     :
                     : "memory");
}

static inline void irq_enable() {
    __asm__ volatile("msr daifclr, #2"
                     :
                     :
                     : "memory");
}

static inline uint64_t irq_save() {
    uint64_t flags;

    __asm__ volatile("mrs %0, daif"
                     : "=&r"(flags)
                     :
                     : "memory");
    irq_disable();

    return flags;
}

static inline void irq_restore(uint64_t flags) {
    __asm__ volatile("msr daif, %0"
                     :
                     : "r"(flags)
                     : "memory");
}
