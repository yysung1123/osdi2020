#pragma once

#include <include/pgtable-types.h>
#include <include/mango_tree.h>

typedef struct {
    pgd_t *pgd;
    struct mango_tree mt;
} mm_struct;

struct vm_area_struct {
    virtaddr_t vm_start;
    virtaddr_t vm_end;
    mm_struct *vm_mm;
    pgprot_t vm_page_prot;
    struct vm_area_struct *alloc_link;
};

#define NVMAS 4096
