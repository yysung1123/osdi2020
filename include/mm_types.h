#pragma once

#include <include/pgtable-types.h>
#include <include/spinlock_types.h>
#include <include/mango_tree.h>
#include <include/types.h>

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
};

struct page_t {
    struct list_head buddy_list;
    atomic_t pp_ref;
    uint64_t private;
};

typedef struct page_t page_t;
