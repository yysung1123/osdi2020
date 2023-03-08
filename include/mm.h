#pragma once

#include <include/memory.h>
#include <include/types.h>
#include <include/atomic.h>
#include <include/mm_types.h>
#include <include/pgtable-types.h>
#include <include/pgtable.h>
#include <include/mango_tree.h>
#include <include/pgtable-hwdef.h>

enum {
    ALLOC_ZERO = 1 << 0,
};

void mem_init();
kernaddr_t boot_alloc(uint32_t);
void buddy_init();
page_t* buddy_alloc(uint8_t);
void buddy_free(page_t *);
void page_init();
page_t* page_alloc(uint32_t);
void page_free(page_t *);
void page_decref(page_t *);
physaddr_t page2pa(page_t *);
page_t* pa2page(physaddr_t);
void pgtable_test();
int32_t __pud_alloc(mm_struct *, pgd_t *, virtaddr_t);
int32_t __pmd_alloc(mm_struct *, pud_t *, virtaddr_t);
int32_t __pte_alloc(mm_struct *, pmd_t *, virtaddr_t);
pte_t* walk_to_pte(mm_struct *, virtaddr_t);
int32_t insert_page(mm_struct *, page_t *, virtaddr_t, pgprot_t);
int32_t follow_pte(mm_struct *, virtaddr_t, pte_t **);
void unmap_page(mm_struct *, virtaddr_t);
void free_pgtables(mm_struct *);
void copy_pgd(mm_struct *, mm_struct *);
void mango_node_init();
struct mango_node* mango_node_alloc();
void mango_node_free(struct mango_node *);
void vma_init();
struct vm_area_struct* vma_alloc();
void vma_free(struct vm_area_struct *);

static inline kernaddr_t page2kva(page_t *pp) {
	return KADDR(page2pa(pp));
}

static inline pgd_t* pgd_alloc() {
    page_t *pp = page_alloc(ALLOC_ZERO);
    pgd_t *pgd = (pgd_t *)page2kva(pp);
    atomic_add(1, &pp->pp_ref);

    return pgd;
}

static inline pud_t* pud_alloc(mm_struct *mm, pgd_t *pgd, virtaddr_t address) {
    return (pgd_none(*pgd) && __pud_alloc(mm, pgd, address)) ?
        NULL : pud_offset(pgd, address);
}

static inline pmd_t* pmd_alloc(mm_struct *mm, pud_t *pud, virtaddr_t address) {
    return (pud_none(*pud) && __pmd_alloc(mm, pud, address)) ?
        NULL : pmd_offset(pud, address);
}

static inline pte_t* pte_alloc(mm_struct *mm, pmd_t *pmd, virtaddr_t address) {
    return (pmd_none(*pmd) && __pte_alloc(mm, pmd, address)) ?
        NULL : pte_offset(pmd, address);
}
