#pragma once

#include <include/pgtable-types.h>
#include <include/spinlock_types.h>
#include <include/mango_tree.h>
#include <include/list.h>

typedef struct {
    pgd_t *pgd;
    struct mango_tree mt;
    spinlock_t mt_lock;
} mm_struct;

struct vm_area_struct {
    virtaddr_t vm_start;
    virtaddr_t vm_end;
    mm_struct *vm_mm;
    pgprot_t vm_page_prot;
    struct list_head alloc_link;
};

#define NVMAS 4096
