SECTIONS
{
  . = 0x80000;
  .text : {
    _KERNEL_START = .;
    *(.text.entry)
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
