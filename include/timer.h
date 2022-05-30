#pragma once

#include <include/mmio.h>

#define EXPIRE_PERIOD 0xfffffff

enum {
    LOCAL_TIMER_CONTROL_REG = ARM_LOCAL_PERIPERHAL_BASE + 0x34,
    LOCAL_TIMER_IRQ_CLR = ARM_LOCAL_PERIPERHAL_BASE + 0x38
};

enum {
    CORE0_TIMER_IRQ_CTRL = ARM_LOCAL_PERIPERHAL_BASE + 0x40,
    CORE0_INTERRUPT_SOURCE = ARM_LOCAL_PERIPERHAL_BASE + 0x60
};

void local_timer_init();
void local_timer_handler();
void core_timer_init();
void core_timer_handler();
