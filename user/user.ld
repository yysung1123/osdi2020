SECTIONS
{
  . = 0x0000000000400000; /* user space */
  .text : {
    *(.text.crt0)
    *(.text .text.*)
  }

  .bss : {
    . = ALIGN(16);
    _BSS_START = .;
    *(.bss .bss.*)
    _BSS_END = .;
  }

  .data : {
    *(.data .data.*)
  }
}
