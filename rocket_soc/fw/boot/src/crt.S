##!  Register 	ABI Name 	Description 			Saver
##!  x0 	zero 		Hard-wired zero 		—
##!  x1 	ra 		Return address 			Caller
##!  x2 	s0/fp 		Saved register/frame pointer 	Callee
##!  x3–13 	s1–11 		Saved registers 		Callee
##!  x14 	sp 		Stack pointer 			Callee
##!  x15 	tp 		Thread pointer 			Callee   
##!  x16–17 	v0–1 		Return values 			Caller
##!  x18–25 	a0–7 		Function arguments 		Caller
##!  x26–30 	t0–4 		Temporaries 			Caller
##!  x31 	gp 		Global 	pointer 		—
##!  f0–15 	fs0–15 		FP saved registers 		Callee
##!  f16–17 	fv0–1 		FP return values 		Caller
##!  f18–25 	fa0–7 		FP arguments 			Caller
##!  f26–31 	ft0–5 		FP temporaries 			Caller

#include "encoding.h"

  .text
  .align 8
  .globl _start
_start:
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

  ##! csrs (pseudo asm instruction) - set bit   
  ##! csrrs - atomic read and set bit
  ##! csrc (pseudo asm instruction) - clear bit 
  ##! csrrc - atomic read and clear bit

  li t0, 0x00001800   # MPP[12:11] = 0x3 (Previous to machine mode)
  csrc mstatus, t0    # run tests in user mode = 0, by clearing bits
  li t0, 0x00000008   # Enable irq in machine and user modes after execution of xRET
  csrs mstatus, t0    # enable interrupts in user mode
  #li t0, MSTATUS_FS;
  #csrs mstatus, t0    # enable FPU
  #li t0, MSTATUS_XS;   
  #csrs mstatus, t0    # enable accelerator

  ##! init mtvec register (see https://github.com/riscv/riscv-test-env/blob/master/p/riscv_test.h)
  la t0, trap_entry
  csrw mtvec, t0
  li t0, 0x00000800
  csrs mie, t0       # Enable External irq (ftom PLIC) for M mode

#if 0
  ##! see https://github.com/riscv/riscv-tests/benchmarks/common
  csrr t0, mstatus
  li t1, MSTATUS_XS
  and t1, t0, t1
  sw t1, have_vec, t2

  ## if that didn't stick, we don't have a FPU, so don't initialize it
  li t1, MSTATUS_FS
  and t1, t0, t1
  beqz t1, 1f
#endif

# intialization when FPU enabled
#ifdef REMOVETHIS__riscv_hard_float
  fssr    x0
  fmv.s.x f0, x0
  fmv.s.x f1, x0
  fmv.s.x f2, x0
  fmv.s.x f3, x0
  fmv.s.x f4, x0
  fmv.s.x f5, x0
  fmv.s.x f6, x0
  fmv.s.x f7, x0
  fmv.s.x f8, x0
  fmv.s.x f9, x0
  fmv.s.x f10,x0
  fmv.s.x f11,x0
  fmv.s.x f12,x0
  fmv.s.x f13,x0
  fmv.s.x f14,x0
  fmv.s.x f15,x0
  fmv.s.x f16,x0
  fmv.s.x f17,x0
  fmv.s.x f18,x0
  fmv.s.x f19,x0
  fmv.s.x f20,x0
  fmv.s.x f21,x0
  fmv.s.x f22,x0
  fmv.s.x f23,x0
  fmv.s.x f24,x0
  fmv.s.x f25,x0
  fmv.s.x f26,x0
  fmv.s.x f27,x0
  fmv.s.x f28,x0
  fmv.s.x f29,x0
  fmv.s.x f30,x0
  fmv.s.x f31,x0
#endif

  ##! initialize global pointer (no need in it)
  lui gp, 0x10000

  ##! get core id
  csrr a0, mhartid            # for now, assume only 1 core
  li a1, 1
1:bgeu a0, a1, 1b

  # Task stack pointer (tp) uses the same value as sp.
#define STACK_512KB 19
  lui t0, 0x10000             # t0 = SRAM base address = 0x10000000 . lui = load upper immediate 20-bits
  add sp, zero, 1             # sp = 1
  sll sp, sp, STACK_512KB     # sp = sp << 19 = 0x80000
  add sp, t0, sp              # sp = sp + 0x10000000 = 0x10080000
  add tp, zero, sp            # tp = sp + 0
  ## Use tp register to save/restore registers context on task switching
  addi tp,tp,-256              # tp = tp - 256 = 0x1007ff00
  # 1 KB for the tap stack + TLS. Add signed extended 12-bits to register
  addi sp, sp, -1024          # sp = 0x1007fc00

  # copy image 64 KB
  jal _init

  ##! jump to entry point in SRAM = 0x10000000
  ##!     'meps' - Machine Exception Program Coutner
  lui t0, 0x10000             # t0 = SRAM base address = 0x10000000 . lui = load upper immediate 20-bits
  csrw mepc, t0
  ##! @see riscv-priv-spec-1.7.pdf. 3.2.1
  ##! After handling a trap, the ERET instruction is used to return to the privilege level at which the
  ##! trap occurred. In addition to manipulating the privilege stack as described in Section 3.1.5, ERET
  ##! sets the pc to the value stored in the Xepc register, where X is the privilege mode (S, H, or M) in
  ##! which the ERET instruction was executed.
  mret

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
  csrc mip, 0x00000800  #csrc pseudo asm instruction clear CSR bit.
                        #[11] MEIP: machine pending external interrupt
  jal handle_trap

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
