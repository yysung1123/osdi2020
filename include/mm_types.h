#pragma once

#include <include/pgtable-types.h>

typedef struct {
    pgd_t *pgd;
} mm_struct;
