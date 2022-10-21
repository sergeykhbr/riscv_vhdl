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
  .globl exception_handler_c
  .globl interrupt_handler_c

_start:
  j reset_vector
trap_table:
  j trap_entry
/* if mtvec is configured as a vectorized interrupt handlers */
  j bad_irq  # 1 Supervisor software interrupt
  j bad_irq  # 2 Reserved
  j bad_irq  # 3 Machine software interrupt
  j bad_irq  # 4 User timer interrupt
  j bad_irq  # 5 Supervisor timer interrupt
  j bad_irq  # 6 Reserved
  j bad_irq  # 7 Machine timer interrupt
  j bad_irq  # 8 User external interrupt
  j bad_irq  # 9 Supervisor external interrupt
  j bad_irq  # 10 Reserved
  j bad_irq  # 11 Machine external interrupt

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

  ##! csrs (pseudo asm instruction) - set bit   
  ##! csrrs - atomic read and set bit
  ##! csrc (pseudo asm instruction) - clear bit 
  ##! csrrc - atomic read and clear bit

  la t0, trap_table
  csrw mtvec, t0

# intialization when HW FPU enabled
#ifdef FPU_ENABLED
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

  ##! get core id
  csrr a0, mhartid            # a0 <= MHARTID value

#define SRAM_BASE_ADDR  0x08000000
#define SRAM_SIZE_BYTES (1<<21)
#define STACK_PER_CORE_BYTES 4096
#define CFG_CPU_MAX 4

  #dedicated region to save cpu context on exception (not interrupt)
  li  sp, SRAM_BASE_ADDR+SRAM_SIZE_BYTES

  # Initialze stack pointer for each cpu
  li  a1, (CFG_CPU_MAX - 1)
  sub a0, a1, a0                  # (CFG_CPU_MAX - 1 - hartid)
  li  a1, STACK_PER_CORE_BYTES
  mul a0, a0, a1
  sub sp, sp, a0
  addi sp,sp,-256
  csrw 0x7b3, sp          # save dedicated stack region into dscratch1 register to run riscv-tests

  # just test amoadd.d (temporary code)
  li a2, 1
  sd a2,-8(sp)
  li a1,8
  sub a1,sp,a1
  amoadd.d zero,a2,(a1)


  # copy image 64 KB
  csrr a0, mhartid            # a0 <= MHARTID value
  beqz a0, init0
  wfi
  j end_init0
init0:
  # HW Initialization is run only on Core[0]
  jal _init
end_init0:

  # a0 = hart id
  # a1 = fdt header
  csrr a0, mhartid
  la a1, dtb_start
  mret

trap_entry:
  ## was it an interrupt or an exception?
  csrw mscratch, t5;
  csrr t5, mcause;
  bgez t5, handle_exception;

interrupt_entry:
  add sp, sp, -COOP_REG_FRAME
  _save_context(sp)

  ## @brief Call function :
  ##       long handle_trap(long cause, long epc, long long regs[32])
  ##             a0 = argument 1: cause
  ##             a1 = argument 2: mepc
  ##             a2 = argument 3: pointer on stack
  ## @return     a0 New instruction pointer offset
  csrr a0, mcause
  csrr a1, mepc
  mv a2, sp
  jal interrupt_handler_c

  # tp-offset in the context array is used to save mepc value. An it may be
  # modified by isr handler during preemtive task switching.
  ld t5,COOP_REG_TP(sp)
  csrw mepc,t5
  _restore_context(sp)
  add sp, sp, COOP_REG_FRAME
  csrr t5,mscratch
  mret


handle_exception:
  # All registers are invalid after riscv-tests finished!!
  csrr sp, 0x7b3          # restore dedicated stack region from dscratch1 dregister
  jal exception_handler_c
  mret


.section .dtb
.align 3
.global dtb_start, dtb_end
dtb_start:
.incbin "virt.dtb"
dtb_end:
