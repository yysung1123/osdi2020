#pragma once

#include <include/pgtable-hwdef.h>
#include <include/pgtable-types.h>
#include <include/types.h>
#include <include/asm/pgtable.h>

static inline pte_t pmd_pte(pmd_t pmd) {
    return __pte(pmd_val(pmd));
}

static inline pte_t pud_pte(pud_t pud) {
    return __pte(pud_val(pud));
}

static inline pte_t pgd_pte(pgd_t pgd) {
    return __pte(pgd_val(pgd));
}

#define __pte_to_phys(pte)	((physaddr_t)(pte_val(pte) & PTE_ADDR_MASK))
#define __pmd_to_phys(pmd)	__pte_to_phys(pmd_pte(pmd))
#define __pud_to_phys(pud)	__pte_to_phys(pud_pte(pud))
#define __pgd_to_phys(pgd)	__pte_to_phys(pgd_pte(pgd))

static inline pte_t* pmd_pgtable(pmd_t *pmd) {
    return (pte_t *)KADDR(__pmd_to_phys(*pmd));
}

static inline pmd_t* pud_pgtable(pud_t *pud) {
    return (pmd_t *)KADDR(__pud_to_phys(*pud));
}

static inline pud_t* pgd_pgtable(pgd_t *pgd) {
    return (pud_t *)KADDR(__pgd_to_phys(*pgd));
}

#define pte_index(a) (((a) >> PTE_SHIFT) & (PTRS_PER_PTE - 1))
#define pmd_index(a) (((a) >> PMD_SHIFT) & (PTRS_PER_PMD - 1))
#define pud_index(a) (((a) >> PUD_SHIFT) & (PTRS_PER_PUD - 1))
#define pgd_index(a) (((a) >> PGDIR_SHIFT) & (PTRS_PER_PGD - 1))

static inline pgd_t* pgd_offset_pgd(pgd_t *pgd, virtaddr_t address) {
    return (pgd + pgd_index(address));
}

static inline pte_t* pte_offset(pmd_t *pmd, virtaddr_t address) {
    return pmd_pgtable(pmd) + (int32_t)pte_index(address);
}

static inline pmd_t* pmd_offset(pud_t *pud, virtaddr_t address) {
    return pud_pgtable(pud) + (int32_t)pmd_index(address);
}

static inline pud_t* pud_offset(pgd_t *pgd, virtaddr_t address) {
    return pgd_pgtable(pgd) + (int32_t)pud_index(address);
}

#define pgd_offset(mm, address) pgd_offset_pgd((mm)->pgd, (address))
