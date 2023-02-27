#pragma once

#include <include/types.h>

struct TrapFrame {
    uint64_t x[31];
    uint64_t esr_el1, elr_el1, spsr_el1, sp_el0;
    uint64_t orig_x0, syscallno;
};

void sync_handler(struct TrapFrame *);
void svc_handler(struct TrapFrame *);
