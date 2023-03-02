#pragma once

#include <include/types.h>

struct Timestamp {
    uint64_t freq, counts;
};

uint64_t timestamp_read_counts();
int64_t do_get_timestamp(struct Timestamp *);
