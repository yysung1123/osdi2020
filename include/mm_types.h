#pragma once

#include <include/pgtable-types.h>
#include <include/spinlock_types.h>

typedef struct {
    pgd_t *pgd;
} mm_struct;
