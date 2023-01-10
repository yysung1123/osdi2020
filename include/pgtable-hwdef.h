#pragma once

#define PAGE_SIZE 4096
#define PAGE_SHIFT 12

#define PGDIR_SHIFT 39
#define PTRS_PER_PGD 512
#define PUD_SHIFT 30
#define PTRS_PER_PUD 512
#define PMD_SHIFT 21
#define PTRS_PER_PMD 512
#define PTE_SHIFT 12
#define PTRS_PER_PTE 512

#define PTE_ADDR_MASK ((pteval_t)((1ull << (48 - PAGE_SHIFT)) - 1) << PAGE_SHIFT)

#define PD_USER (1ull << 6)
#define PD_RW (PD_USER)
#define PD_RO (PD_USER | (1ull << 7))
#define PD_NX (1ull << 54)

#define PGD_TYPE_TABLE PD_TABLE
#define PUD_TYPE_TABLE PD_TABLE
#define PMD_TYPE_TABLE PD_TABLE
#define PTE_ATTR_NORMAL (PD_ACCESS | (MAIR_IDX_NORMAL_NOCACHE << 2) | PD_TABLE)
