#include "encoding.h"

##! Disabling the compressed code
  .option norvc

  .section .isr_vector, "x"
  .align 4
  .globl _start
  .globl _mbist_ram_exit
  .globl exception_handler_c
  .globl interrupt_handler_c
  .globl interrupt_s_software_c
  .globl interrupt_m_software_c
  .globl interrupt_s_timer_c
  .globl interrupt_m_timer_c
  .globl interrupt_s_external_c
  .globl interrupt_m_external_c

_start:
  j reset_vector
trap_table:
  j trap_entry
/* if mtvec is configured as a vectorized interrupt handlers */
  j interrupt_s_software_entry  # 1 Supervisor software interrupt
  j bad_irq                     # 2 Reserved
  j interrupt_m_software_entry  # 3 Machine software interrupt
  j bad_irq                     # 4 User timer interrupt
  j interrupt_s_timer_entry     # 5 Supervisor timer interrupt
  j bad_irq                     # 6 Reserved
  j interrupt_m_timer_entry     # 7 Machine timer interrupt
  j bad_irq                     # 8 User external interrupt
  j interrupt_s_external_entry  # 9 Supervisor external interrupt
  j bad_irq                     # 10 Reserved
  j interrupt_m_external_entry  # 11 Machine external interrupt

bad_irq:
  j bad_irq

reset_vector:
  li  x1, 0
  li  x2, 0
  li  x3, 0
  li  x4, 0
  li  x5, 0
  li  x6, 0
  li  x7, 0
  li  x8, 0
  li  x9, 0
  li  x10,0
  li  x11,0
  li  x12,0
  li  x13,0
  li  x14,0
  li  x15,0
  li  x16,0
  li  x17,0
  li  x18,0
  li  x19,0
  li  x20,0
  li  x21,0
  li  x22,0
  li  x23,0
  li  x24,0
  li  x25,0
  li  x26,0
  li  x27,0
  li  x28,0
  li  x29,0
  li  x30,0
  li  x31,0
# intialization when HW FPU enabled
#ifdef FPU_ENABLED
  fssr    x0
  fmv.d.x f0, x0
  fmv.d.x f1, x0
  fmv.d.x f2, x0
  fmv.d.x f3, x0
  fmv.d.x f4, x0
  fmv.d.x f5, x0
  fmv.d.x f6, x0
  fmv.d.x f7, x0
  fmv.d.x f8, x0
  fmv.d.x f9, x0
  fmv.d.x f10,x0
  fmv.d.x f11,x0
  fmv.d.x f12,x0
  fmv.d.x f13,x0
  fmv.d.x f14,x0
  fmv.d.x f15,x0
  fmv.d.x f16,x0
  fmv.d.x f17,x0
  fmv.d.x f18,x0
  fmv.d.x f19,x0
  fmv.d.x f20,x0
  fmv.d.x f21,x0
  fmv.d.x f22,x0
  fmv.d.x f23,x0
  fmv.d.x f24,x0
  fmv.d.x f25,x0
  fmv.d.x f26,x0
  fmv.d.x f27,x0
  fmv.d.x f28,x0
  fmv.d.x f29,x0
  fmv.d.x f30,x0
  fmv.d.x f31,x0
#endif
  csrw mscratch, x0

  ##! init mtvec register in vector mode (MODE=0x1)
  la t0, trap_table
  or t0, t0, 0x1
  csrw mtvec, t0

  ##! get core id
  csrr a0, mhartid            # a0 <= MHARTID value

  # Task stack pointer (tp) uses the same value as sp.
#define SRAM_BASE_ADDR  0x08000000
#define SRAM_SIZE_BYTES (1<<18)
#define STACK_PER_CORE_BYTES 4096
#define CFG_CPU_MAX 4

  li  sp, SRAM_BASE_ADDR+SRAM_SIZE_BYTES

  # Initialze stack pointer for each cpu
  li  a1, (CFG_CPU_MAX - 1)
  sub a0, a1, a0                  # (CFG_CPU_MAX - 1 - hartid)
  li  a1, STACK_PER_CORE_BYTES
  mul a0, a0, a1
  sub sp, sp, a0
  # allocate special 256B region in a stack to save/restore context
  addi sp, sp,-256
  csrw 0x7b3, sp          # save dedicated stack region into dscratch1 register to run riscv-tests
  mv tp, sp               # or use dedicated tp for context saving


  # check Hart ID
  csrr a0, mhartid            # a0 <= MHARTID value
  beqz a0, init0
  j end_init0
init0:
  # HW Initialization is run only on Core[0]
  # run memory selftest only on Core[0]
  #jal _mbist_ram
_mbist_ram_exit:
end_init0:

  # run tests in machine mode:
  jal main
  ## Never reach here



interrupt_s_software_entry:
  _save_context(tp)
  jal interrupt_s_software_c
  _restore_context(tp)
  sret

interrupt_m_software_entry:
  _save_context(tp)
  jal interrupt_m_software_c
  _restore_context(tp)
  mret


interrupt_s_timer_entry:
  _save_context(tp)
  jal interrupt_s_timer_c
  _restore_context(tp)
  sret

interrupt_m_timer_entry:
  _save_context(tp)
  jal interrupt_m_timer_c
  _restore_context(tp)
  mret


interrupt_s_external_entry:
  _save_context(tp)
  jal interrupt_s_external_c
  _restore_context(tp)
  sret

interrupt_m_external_entry:
  _save_context(tp)
  jal interrupt_m_external_c
  _restore_context(tp)
  mret

trap_entry:
  ## was it an interrupt or an exception?
  csrw mscratch, t5;
  csrr t5, mcause;
  bgez t5, handle_exception;

interrupt_entry:
  _save_context(tp)

  ## @brief Call function :
  ##       long handle_trap(long cause, long epc, long long regs[32])
  ##             a0 = argument 1: cause
  ##             a1 = argument 2: mepc
  ##             a2 = argument 3: pointer on stack
  ## @return     a0 New instruction pointer offset
  csrr a0, mcause
  csrr a1, mepc
  sd a1,COOP_REG_TP(tp)
  mv a2, sp
  jal interrupt_handler_c

  # tp-offset in the context array is used to save mepc value. An it may be
  # modified by isr handler during preemtive task switching.
  ld t5,COOP_REG_TP(tp)
  csrw mepc,t5
  _restore_context(tp)
  csrr t5,mscratch
  mret

handle_exception:
  _save_context(tp)
  jal exception_handler_c
  _restore_context(tp)
  csrr t5, mscratch;
  mret

.section .dtb
.align 3
.global dtb_start, dtb_end
dtb_start:
.incbin "virt.dtb"
dtb_end:
