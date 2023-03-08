#pragma once

#include <include/types.h>
#include <include/spinlock_types.h>

#define MAX_ORDER 19 // 4k ~ 1G

struct free_area {
    struct list_head free_list;
    uint64_t nr_free;
};

struct buddy_system {
    struct free_area free_area[MAX_ORDER];
    spinlock_t lock;
};
