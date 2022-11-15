SECTIONS
{
  . = 0xffff000000000000; /* kernel space */
  . += 0x80000; /* kernel load address */
  .text : {
    _KERNEL_START = .;
    *(.text.start)
    *(.text .text.*)
  }

  .bss : {
    . = ALIGN(16);
    _BSS_START = .;
    *(.bss .bss.*)
    _BSS_END = .;
  }

  . = ALIGN(0x1000);
  pg_dir = .;
  .data.pg : {
    /* (1 * PGD) + (1 * PUD) + (1 * PMD) */
    . += (1 + 1 + 1) * 0x1000;
  }
  _KERNEL_END = .;
}

_KERNEL_SIZE = _KERNEL_END - _KERNEL_START;
