#pragma once

#include <include/types.h>

typedef enum {
    ATOMIC_ADD = 0
} TEST_OP;

int64_t do_test(TEST_OP);
