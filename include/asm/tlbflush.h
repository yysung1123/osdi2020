#pragma once

#include <include/asm/barrier.h>

static inline void tlbi_vmalle1is() {
    __asm__ volatile("tlbi vmalle1is" : : : "memory");
}

static inline void flush_tlb_all() {
	dsb(ishst);
	tlbi_vmalle1is();
	dsb(ish);
	isb();
}
