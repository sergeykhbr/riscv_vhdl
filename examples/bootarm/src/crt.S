#http://www.ic.unicamp.br/~celio/mc404-2014/docs/gnu-arm-directives.pdf
# R11 = FP (frame pointer)
# R13 = SP
# R14 = LR (Link Register). Register R14 receives the return address when a Branch with Link (BL or BLX) instruction is executed.
# R15 = PC
#==================================================================
# Entry point for the Reset handler
#==================================================================
#    .section INTERRUPT_VECTOR, "x"
     .text
_Reset:

#==================================================================
# Exception Vector Table
#==================================================================

Vectors:
        LDR PC, Reset_Addr
        LDR PC, Undefined_Addr  @ Undefined Instruction
        LDR PC, SVC_Addr        @ Software interrupt
        LDR PC, Prefetch_Addr   @ Abort (prefetch)
        LDR PC, Abort_Addr      @ Abort (data)
        B .                     @ Reserved vector
        LDR PC, IRQ_Addr        @ IRQ
        LDR PC, FIQ_Addr        @ FIQ


    .balign 4
Reset_Addr:     .word Reset_Handler
Undefined_Addr: .word Undefined_Handler
SVC_Addr:       .word SVC_Handler
Prefetch_Addr:  .word Prefetch_Handler
Abort_Addr:     .word Abort_Handler
IRQ_Addr:       .word 0x00000000
FIQ_Addr:       .word 0x00000000

# 512 KB RAM
.equ STACK_BASE,        0x10080000

#==================================================================
# Exception Handlers
#==================================================================

Undefined_Handler:
        B   Undefined_Handler
SVC_Handler:
        B   SVC_Handler
Prefetch_Handler:
        B   Prefetch_Handler
Abort_Handler:
        B   Abort_Handler

FIQ_Handler:
        B   FIQ_Handler


#==================================================================
# Reset Handler
#==================================================================

#    .global Reset_Handler
#    .type Reset_Handler, "function"
Reset_Handler:
#==================================================================
# Disable MPU and caches
#==================================================================

# Disable MPU and cache in case it was left enabled from an earlier run
# This does not need to be done from a cold reset

        MRC     p15, 0, r0, c1, c0, 0       @ Read System Control Register
        BIC     r0, r0, #0x05               @ Disable MPU (M bit) and data cache (C bit)
        BIC     r0, r0, #0x1000             @ Disable instruction cache (I bit)
        DSB                                 @ Ensure all previous loads/stores have completed
        MCR     p15, 0, r0, c1, c0, 0       @ Write System Control Register
        ISB                                 @ Ensure subsequent insts execute wrt new MPU settings

#==================================================================
# Disable Branch prediction
#==================================================================

# In the Cortex-R5, the Z-bit of the SCTLR does not control the program flow prediction.
# Some control bits in the ACTLR control the program flow and prefetch features instead.
# These are enabled by default, but are shown here for completeness.

        MRC     p15, 0, r0, c1, c0, 1       @ Read ACTLR
        ORR     r0, r0, #(0x1 << 17)        @ Enable RSDIS bit 17 to disable the return stack
        ORR     r0, r0, #(0x1 << 16)        @ Clear BP bit 15 and set BP bit 16:
        BIC     r0, r0, #(0x1 << 15)        @ Branch always not taken and history table updates disabled
        MCR     p15, 0, r0, c1, c0, 1       @ Write ACTLR

# Bits M[4:0] in CPSR register
.equ Mode_User,   0x10
.equ Mode_FIQ,    0x11
.equ Mode_IRQ,    0x12
.equ Mode_SVC,    0x13  @ supervisor (after reset)
.equ Mode_ABORT,  0x17
.equ Mode_UNDEF,  0x1B
.equ Mode_SYSTEM, 0x1F
# The I and F bits are the interrupt disable bits
.equ F_Bit,   0x40   @ FIQ disable
.equ I_Bit,   0x80   @ IRQ disable

       LDR r10,=0x0
       LDR r11,=0x0
       LDR r12,=0x0
       LDR r13,=0x0
       LDR r14,=0x0 @ lp

       LDR  r0, =(STACK_BASE-16)
# @note:
#   MSR     CPSR_c, xxxx      ; sets the control bits
#   MSR     CPSR_f, xxxx      ; sets the flag bits
#   MSR     CPSR_cxsf, xxxx   ; sets everything
#   MRS     R0, CPSR          ; Copy CPSR into R0

       # Init FIQ stack pointer
       MSR     CPSR_c, #(Mode_FIQ | I_Bit | F_Bit)    @ Interrupts disabled
       MOV     SP, R0
       MOV     FP, R0
       #SUB     R0, R0, #Len_FIQ_Stack
       # Init IRQ stack pointer
       MSR     CPSR_c, #(Mode_IRQ | I_Bit | F_Bit)    @ Interrupts disabled
       MOV     SP, R0
       #SUB     R0, R0, #Len_IRQ_Stack
       # Init IRQ stack pointer (default = Superviser mode)
       MSR     CPSR_c, #(Mode_SVC | I_Bit | F_Bit)    @ Interrupts disabled
       MOV     SP, R0
       MOV     FP, R0
       

#==================================================================
# Cache invalidation
#==================================================================
        DSB                 @ Complete all outstanding explicit memory operations
        MOV     r0, #0
        MCR     p15, 0, r0, c7, c5, 0       @ Invalidate entire instruction cache
        MCR     p15, 0, r0, c15, c5, 0      @ Invalidate entire data cache

        B       0x10000000
    .size Reset_Handler, . - Reset_Handler

