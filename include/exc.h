#pragma once

#include <include/types.h>

struct TrapFrame {
    uint64_t x[31];
    uint64_t esr_el1, elr_el1, spsr_el1;
};

void sync_handler(struct TrapFrame *);
void svc_handler(struct TrapFrame *);
