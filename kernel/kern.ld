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
  _KERNEL_END = .;
}

_KERNEL_SIZE = _KERNEL_END - _KERNEL_START;
