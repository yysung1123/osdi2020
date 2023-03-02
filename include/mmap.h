#pragma once

#include <include/types.h>

typedef enum {
    PROT_NONE = 0x0,
    PROT_READ = 0x1,
    PROT_WRITE = 0x2,
    PROT_EXEC = 0x4
} mmap_prot_t;

typedef enum {
    MAP_FIXED = 0x10,
    MAP_ANONYMOUS = 0x20,
    MAP_POPULATE = 0x008000
} mmap_flags_t;

void* do_mmap(void *, size_t, mmap_prot_t, mmap_flags_t, void *, off_t);
