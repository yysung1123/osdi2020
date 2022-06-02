#pragma once

#include <include/types.h>

struct Timestamp {
    uint64_t freq, counts;
};

int64_t do_get_timestamp(struct Timestamp *);
