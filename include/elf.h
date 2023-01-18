#pragma once

#include <include/types.h>

#define ELF_MAGIC 0x464c457f
#define EI_NIDENT 16

enum {
    PT_LOAD = 1
};

enum {
    PF_X = 0x1,
    PF_W = 0x2,
    PF_R = 0x4
};

typedef struct elf64_hdr {
    unsigned char e_ident[EI_NIDENT];	/* ELF "magic number" */
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;		/* Entry point virtual address */
    uint64_t e_phoff;		/* Program header table file offset */
    uint64_t e_shoff;		/* Section header table file offset */
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf64_Ehdr;

typedef struct elf64_phdr {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;		/* Segment file offset */
    uint64_t p_vaddr;		/* Segment virtual address */
    uint64_t p_paddr;		/* Segment physical address */
    uint64_t p_filesz;		/* Segment size in file */
    uint64_t p_memsz;		/* Segment size in memory */
    uint64_t p_align;		/* Segment alignment, file & memory */
} Elf64_Phdr;
