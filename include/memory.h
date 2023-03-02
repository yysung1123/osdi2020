#pragma once

#include <include/pgtable-hwdef.h>
#include <include/types.h>
#include <include/assert.h>

#define MEMSIZE (1 << 30) // 1GB RAM
#define KERNBASE 0xffff000000000000
#define NPAGES (MEMSIZE / PAGE_SIZE)

#define PGNUM(pa) (((uint64_t)(pa)) >> PAGE_SHIFT)
#define VPGNUM(va) (((uint64_t)(va)) >> PAGE_SHIFT)

#define VPGNUM2VA(vpgnum) (((virtaddr_t)(vpgnum))<< PAGE_SHIFT)

#define PADDR(kva) _paddr(__FILE__, __LINE__, kva)
#define KADDR(pa) _kaddr(__FILE__, __LINE__, pa)

#define PAGE_ALIGN(addr) (ROUNDUP(addr, PAGE_SIZE))

static inline physaddr_t _paddr(const char *file, int line, kernaddr_t kva) {
	if ((uint64_t)kva < KERNBASE) {
		_panic(file, line, "PADDR called with invalid kva %016lx", kva);
    }
	return (physaddr_t)kva - KERNBASE;
}

static inline physaddr_t _kaddr(const char *file, int line, physaddr_t pa) {
    if (PGNUM(pa) >= NPAGES) {
		_panic(file, line, "KADDR called with invalid pa %016lx", pa);
    }
	return (physaddr_t)(pa + KERNBASE);
}
