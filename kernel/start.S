.section ".text.start"
_start:
  mrs x0, mpidr_el1
  and x0, x0, 3
  cbz x0, 2f
1:
  wfe
  b 1b
2:
// Initialize the BSS segment
  ldr x1, = _BSS_START
  ldr x2, = _BSS_END
3:
  cmp x1, x2
  bge 4f
  str xzr, [x1]
  add x1, x1, 8
  b 3b
4:
// EL2 to EL1
  mov x0, xzr
  msr SCTLR_EL1, x0 // disable MMU
  mov x0, #0x00300000
  msr CPACR_EL1, x0 // enable FP and SIMD
  mov x0, #0x80000000
  msr HCR_EL2, x0 // The Execution state for EL1 is AArch64
  mov x0, #5
  msr SPSR_EL2, x0 // EL1h
  ldr x0, = el1_start
  msr ELR_EL2, x0
  eret
el1_start:
// Set the stack pointer
  ldr x0, = _stack_top
  mov sp, x0
// load exception_table to VBAR_EL1
  ldr x0, = exception_table
  msr VBAR_EL1, x0
  bl main
loop:
  b loop

.bss
  .globl _stack
_stack:
  .space 8 * 4096
  .globl _stack_top
_stack_top:
