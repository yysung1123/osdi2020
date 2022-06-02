#pragma once

#include <include/types.h>

struct TrapFrame {
    uint64_t x[31];
};

void sync_handler(struct TrapFrame *);
void svc_handler(struct TrapFrame *);
