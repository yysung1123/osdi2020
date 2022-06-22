#pragma once

#include <include/types.h>

typedef uint64_t sigset_t;

enum {
    SIGKILL = 9
};

void check_signal();
int64_t do_kill(int32_t, uint8_t);
