
#include "encoding.h"

  .text
  .align 8

  .globl init_mtvec
init_mtvec:
  ##! init mtvec register (see https://github.com/riscv/riscv-test-env/blob/master/p/riscv_test.h)
  la t0, trap_entry
  csrw mtvec, t0
  li t0, 0x00000800
  csrs mie, t0       # Enable External irq (ftom PLIC) for M mode
  ret


trap_entry:
  ##! module CSRFile rises io_fatc signal that is cause of the 'ptw.invalidate'.
  fence
  csrw mscratch, a0;

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
  # !!! Cannot reset external pending bits only via IrqController (page 28)
  li t0, 0x00000800
  csrc mip, t0      #csrc pseudo asm instruction clear CSR bit.
                    #[11] MEIP: machine pending external interrupt
  jal isr_example_c

  # tp-offset in the context array is used to save mepc value. An it may be
  # modified by isr handler during preemtive task switching.
  ld a1,COOP_REG_TP(tp)
  csrw mepc,a1
  _restore_context(tp)
  mret

.section ".tdata.begin"
.globl _tdata_begin
_tdata_begin:

.section ".tdata.end"
.globl _tdata_end
_tdata_end:

.section ".tbss.end"
.globl _tbss_end
_tbss_end:
