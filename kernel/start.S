#include <include/asm/pgtable.h>

.section ".text.start"
_start:
  mrs x0, mpidr_el1
  and x0, x0, 3
  cbz x0, 2f
1:
  wfe
  b 1b
2:
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
  and x0, x0, 0x0000ffffffffffff // convert el1_start from upper address space to lower address space
  msr ELR_EL2, x0
  eret
el1_start:
// set up TCR_EL1
  ldr x0, = TCR_CONFIG_DEFAULT
  msr TCR_EL1, x0
// set up MAIR_EL1
  ldr x0, =( \
    (MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) | \
    (MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8)) \
  )
  msr MAIR_EL1, x0
// set up finer granularity kernel mapping
  bl page_table_create

  mrs x2, SCTLR_EL1
  orr x2, x2, 1
  msr SCTLR_EL1, x2 // enable MMU, cache remains disabled

  ldr x2, = boot_rest // indirect branch to the virtual address
  br x2
boot_rest:
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
// load exception_table to VBAR_EL1
  ldr x0, = exception_table
  msr VBAR_EL1, x0
// Set the stack pointer
  ldr x0, = _el1_stack_top
  mov sp, x0
  bl main
loop:
  b loop

page_table_create:
  ldr x0, = pg_dir
  and x0, x0, 0x0000ffffffffffff // convert PGD from upper address space to lower address space
  add x1, x0, 0x1000 // PUD
  add x2, x0, 0x2000 // PMD

  // the first entry of PGD -> PUD
  ldr x3, = BOOT_PGD_ATTR
  orr x3, x1, x3 // combine the physical address of next level page with attribute.
  str x3, [x0]

  // the first entry of PUD -> PMD
  ldr x3, = BOOT_PUD_ATTR
  orr x3, x3, x2
  str x3, [x1]

  // the second entry of PUD -> normal block
  ldr x3, = BOOT_PUD_ATTR_BLOCK
  mov x4, 0x40000000 // block's physical address
  orr x3, x3, x4
  str x3, [x1, 8]

  // the [0, 512 - 8)th entries of PMD -> normal block
  mov x4, xzr
pmd_normal_block_create_start:
  cmp x4, (512 - 8)
  bge pmd_normal_block_create_end
  ldr x3, = BOOT_PMD_ATTR_NORMAL
  lsl x5, x4, 21 // block's physical address, x4 * 2MB
  orr x3, x3, x5
  str x3, [x2, x4, lsl#3]
  add x4, x4, 1
  b pmd_normal_block_create_start
pmd_normal_block_create_end:

  // the [512 - 8, 512)th entries of PMD -> device block
  mov x4, 512 - 8
pmd_device_block_create_start:
  cmp x4, 512
  bge pmd_device_block_create_end
  ldr x3, = BOOT_PMD_ATTR_DEVICE
  lsl x5, x4, 21 // block's physical address, x4 * 2MB
  orr x3, x3, x5
  str x3, [x2, x4, lsl#3]
  add x4, x4, 1
  b pmd_device_block_create_start
pmd_device_block_create_end:

  msr TTBR0_EL1, x0 // load PGD to the buttom translation based register.
  msr TTBR1_EL1, x0 // also load PGD to the upper translation based register.
  ret

.bss
  .align 12
  .globl _el1_stack
_el1_stack:
  .space 4096
  .globl _el1_stack_top
_el1_stack_top:

  .align 12
  .globl _interrupt_stack
_interrupt_stack:
  .space 4096
  .globl _interrupt_stack_top
_interrupt_stack_top:
