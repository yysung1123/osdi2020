SECTIONS
{
  . = 0x80000;
  .text : { *(.text.entry) }

  .bss : {
    . = ALIGN(16);
    _BSS_START = .;
    *(.bss .bss.*)
    _BSS_END = .;
  }
}
