.section ".text.crt0"

.globl _start

_start:
  bl main
  bl exit
