// Simple vector table

.macro kernel_entry, el
  stp x0, x1, [sp, #16 * 0]
  stp x2, x3, [sp, #16 * 1]
  stp x4, x5, [sp, #16 * 2]
  stp x6, x7, [sp, #16 * 3]
  stp x8, x9, [sp, #16 * 4]
  stp x10, x11, [sp, #16 * 5]
  stp x12, x13, [sp, #16 * 6]
  stp x14, x15, [sp, #16 * 7]
  stp x16, x17, [sp, #16 * 8]
  stp x18, x19, [sp, #16 * 9]
  stp x20, x21, [sp, #16 * 10]
  stp x22, x23, [sp, #16 * 11]
  stp x24, x25, [sp, #16 * 12]
  stp x26, x27, [sp, #16 * 13]
  stp x28, x29, [sp, #16 * 14]
  mrs x0, esr_el1
  mrs x1, elr_el1
  mrs x2, spsr_el1
  stp x30, x0, [sp, #16 * 15]
  stp x1, x2, [sp, #16 * 16]
.endm

.macro kernel_exit, el
  ldp x1, x2, [sp, #16 * 16]
  ldp x30, x0, [sp, #16 * 15]
  msr spsr_el1, x2
  msr elr_el1, x1
  msr esr_el1, x0
  ldp x28, x29, [sp, #16 * 14]
  ldp x26, x27, [sp, #16 * 13]
  ldp x24, x25, [sp, #16 * 12]
  ldp x22, x23, [sp, #16 * 11]
  ldp x20, x21, [sp, #16 * 10]
  ldp x18, x19, [sp, #16 * 9]
  ldp x16, x17, [sp, #16 * 8]
  ldp x14, x15, [sp, #16 * 7]
  ldp x12, x13, [sp, #16 * 6]
  ldp x10, x11, [sp, #16 * 5]
  ldp x8, x9, [sp, #16 * 4]
  ldp x6, x7, [sp, #16 * 3]
  ldp x4, x5, [sp, #16 * 2]
  ldp x2, x3, [sp, #16 * 1]
  ldp x0, x1, [sp, #16 * 0]
  add sp, sp, #16 * 17
  eret
.endm

.macro entry_handler el:req label:req
  sub sp, sp, #16 * 17
  kernel_entry \el
// set handler argument
  mov x0, sp
// set interrupt stack
  .if \label == irq
  ldr x1, = _interrupt_stack_top
  mov sp, x1
  str x0, [sp, #-16]!
  mov x0, xzr
  .endif
  bl \label\()_handler
// restore stack
  .if \label == irq
  ldr x0, [sp], #16
  mov sp, x0
  .endif
  .if \el == 0
  b ret_to_user
  .else
  b ret_to_kernel
  .endif
.endm

.macro exception_handler el
  sub sp, sp, #16 * 17
  kernel_entry \el
  mov x0, sp
  bl print_exception_info
  .if \el == 0
  b ret_to_user
  .else
  b ret_to_kernel
  .endif
.endm

// kernel_entry + kernel_exit > entry size
// hence, move kernel_exit out of entry_handler
ret_to_user:
  kernel_exit 0

ret_to_kernel:
  kernel_exit 1

.align 11 // vector table should be aligned to 0x800
.global exception_table
exception_table:
  exception_handler 1 // branch to a handler function.
  .align 7 // entry size is 0x80, .align will pad 0
  exception_handler 1
  .align 7
  exception_handler 1
  .align 7
  exception_handler 1
  .align 7

  exception_handler 1
  .align 7
  entry_handler 1, irq
  .align 7
  exception_handler 1
  .align 7
  exception_handler 1
  .align 7

  entry_handler 0, sync
  .align 7
  entry_handler 0, irq
  .align 7
  exception_handler 0
  .align 7
  exception_handler 0
  .align 7

  exception_handler 0
  .align 7
  exception_handler 0
  .align 7
  exception_handler 0
  .align 7
  exception_handler 0
  .align 7

.global get_current
get_current:
  mrs x0, tpidr_el1
  ret
