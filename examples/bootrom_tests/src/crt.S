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

##! Disabling the compressed code
  .option norvc

  .section .isr_vector, "x"
  .align 4
  .globl _start
  .globl _mbist_ram_exit
  .globl exception_handler_c
  .globl interrupt_handler_c

# 8-bytes per exception entry
trap_table:
  j _start            # 
  nop
  j exception_entry   # Instruction Misaligned
  nop
  j exception_entry   # Instruction Load Fault
  nop
  j exception_entry   # Instruction Illegal
  nop
  j bad_trap          # Breakpoint
  nop
  j exception_entry   # Load Misaligned
  nop
  j exception_entry   # Load Fault
  nop
  j exception_entry   # Store Misaligned
  nop
  j exception_entry   # Store Fault
  nop
  j exception_entry   # Call from U-mode
  nop
  j bad_trap          # Call from S-mode
  nop
  j bad_trap          # Call from H-mode
  nop
  j exception_entry   # Call from M-mode
  nop
  j exception_entry   # Instruction Page Fault
  nop
  j exception_entry   # Load Page Fault
  nop
  j exception_entry   # NMI14_Reserved
  nop
  j exception_entry   # Store Page Fault
  nop
  j exception_entry   # Stack Overflow
  nop
  j exception_entry   # Stack Underflow
  nop

bad_trap:
  j bad_trap

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
  la t0, interrupt_entry
  csrw mtvec, t0
  li t0, 0x00000800
  csrs mie, t0       # Enable External irq (ftom PLIC) for M mode

#if 0
  ##! see https://github.com/riscv/riscv-tests/benchmarks/common
  csrr t0, mstatus
  li t1, MSTATUS_XS
  and t1, t0, t1
  sw t1, have_vec, t2
#endif
#if 0
  ## if that didn't stick, we don't have a FPU, so don't initialize it
  li t1, MSTATUS_FS
  and t1, t0, t1
  beqz t1, 1f
#endif

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

  ##! initialize global pointer (no need in it)
  lui gp, 0x10000

  ##! get core id
  csrr a0, mhartid            # a0 <= MHARTID value

  # Task stack pointer (tp) uses the same value as sp.
#define SRAM_BASE_ADDR  0x10000000
#define SRAM_SIZE_BYTES (1<<19)
#define STACK_CORE1_BYTES 4096

  li  sp, SRAM_BASE_ADDR+SRAM_SIZE_BYTES
  li  a1, 1
  beq a0, a1, sp_init_core1
  li  a1, 0
  beq a0, a1, sp_init_core0
sp_init_core1:
  j sp_init_coreall
sp_init_core0:
  li t0,-STACK_CORE1_BYTES
  add sp, sp, t0

  # run memory selftest only on Core[0]
  #jal _mbist_ram
_mbist_ram_exit:

sp_init_coreall:
  add tp, zero, sp            # tp = sp + 0 (mov)
  ## Use tp register to save/restore registers context on task switching
  addi tp,tp,-256              # tp = tp - 256 = 0x1007ff00
  addi sp, sp,-264

  jal main
  ## Never reach here

exception_entry:
  _save_context(tp)
  jal exception_handler_c
  _restore_context(tp)
  mret

interrupt_entry:
  ##! module CSRFile rises io_fatc signal that is cause of the 'ptw.invalidate'.
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
  jal interrupt_handler_c

  # tp-offset in the context array is used to save mepc value. An it may be
  # modified by isr handler during preemtive task switching.
  ld a1,COOP_REG_TP(tp)
  csrw mepc,a1
  _restore_context(tp)
  mret

