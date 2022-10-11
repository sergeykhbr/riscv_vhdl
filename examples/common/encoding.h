// See LICENSE for license details.

#ifndef RISCV_CSR_ENCODING_H
#define RISCV_CSR_ENCODING_H

/** Table of non-maskable exceptions */
#define EXCEPTION_InstrMisalign 0    // Instruction misalgined load
#define EXCEPTION_InstrFault    1    // Instruction access fault
#define EXCEPTION_InstrIllegal  2    // Illegal instruction
#define EXCEPTION_Breakpoint    3    // Breakpoint
#define EXCEPTION_LoadMisalign  4    // Load address misaligned
#define EXCEPTION_LoadFault     5    // Load access fault
#define EXCEPTION_StoreMisalign 6    // Store/AMO address misaligned
#define EXCEPTION_StoreFault    7    // Store/AMO access fault
#define EXCEPTION_CallFromUmode 8    // Environment call from U-mode
#define EXCEPTION_CallFromSmode 9    // Environment call from S-mode
#define EXCEPTION_CallFromHmode 10   // Environment call from H-mode
#define EXCEPTION_CallFromMmode 11   // Environment call from M-mode
#define EXCEPTION_InstrPageFault 12
#define EXCEPTION_LoadPageFault  13
#define EXCEPTION_reserved14     14
#define EXCEPTION_StorePageFault 15
#define EXCEPTION_StackOverflow  16   // Stack overflow
#define EXCEPTION_StackUnderflow 17  // Stack overflow
#define EXCEPTION_Total 18           // Table size

#define SLL32    sllw
#define STORE    sd
#define LOAD     ld
#define LWU      lwu
#define LOG_REGBYTES 3
#define REGBYTES (1 << LOG_REGBYTES)


/** Return address */
#define COOP_REG_RA         0//(0*sizeof(uint64_t))
/** Saved registers */
#define COOP_REG_S0         8//(1*sizeof(uint64_t))
#define COOP_REG_S1         16//(2*sizeof(uint64_t))
#define COOP_REG_S2         24//(3*sizeof(uint64_t))
#define COOP_REG_S3         32//(4*sizeof(uint64_t))
#define COOP_REG_S4         40//(5*sizeof(uint64_t))
#define COOP_REG_S5         48//(6*sizeof(uint64_t))
#define COOP_REG_S6         56//(7*sizeof(uint64_t))
#define COOP_REG_S7         64//(8*sizeof(uint64_t))
#define COOP_REG_S8         72//(9*sizeof(uint64_t))
#define COOP_REG_S9         80//(10*sizeof(uint64_t))
#define COOP_REG_S10        88//(11*sizeof(uint64_t))
#define COOP_REG_S11        96//(12*sizeof(uint64_t))
/** Stack pointer */
#define COOP_REG_SP         104//(13*sizeof(uint64_t))
/** Thread pointer */
#define COOP_REG_TP         112//(14*sizeof(uint64_t))
/** Return values */
#define COOP_REG_V0         120//(15*sizeof(uint64_t))
#define COOP_REG_V1         128//(16*sizeof(uint64_t))
/** Function Arguments */
#define COOP_REG_A0         136//(17*sizeof(uint64_t))
#define COOP_REG_A1         144//(18*sizeof(uint64_t))
#define COOP_REG_A2         152//(19*sizeof(uint64_t))
#define COOP_REG_A3         160//(20*sizeof(uint64_t))
#define COOP_REG_A4         168//(21*sizeof(uint64_t))
#define COOP_REG_A5         176//(22*sizeof(uint64_t))
#define COOP_REG_A6         184//(23*sizeof(uint64_t))
#define COOP_REG_A7         192//(24*sizeof(uint64_t))
/** Temporary registers */
#define COOP_REG_T0         200//(25*sizeof(uint64_t))
#define COOP_REG_T1         208//(26*sizeof(uint64_t))
#define COOP_REG_T2         216//(27*sizeof(uint64_t))
#define COOP_REG_T3         224//(28*sizeof(uint64_t))
#define COOP_REG_T4         232//(29*sizeof(uint64_t))
/** Global pointer */
#define COOP_REG_GP         240//(30*sizeof(uint64_t))
#define COOP_REG_FRAME      248//(30*sizeof(uint64_t))

#define INTEGER_CONTEXT_SIZE (32*REGBYTES)

#define _save_context(TO) \
  sd ra, COOP_REG_RA(TO); \
  sd s0, COOP_REG_S0(TO); \
  sd s1, COOP_REG_S1(TO); \
  sd s2, COOP_REG_S2(TO); \
  sd s3, COOP_REG_S3(TO); \
  sd s4, COOP_REG_S4(TO); \
  sd s5, COOP_REG_S5(TO); \
  sd s6, COOP_REG_S6(TO); \
  sd s7, COOP_REG_S7(TO); \
  sd s8, COOP_REG_S8(TO); \
  sd s9, COOP_REG_S9(TO); \
  sd s10, COOP_REG_S10(TO); \
  sd s11, COOP_REG_S11(TO); \
  sd sp, COOP_REG_SP(TO); \
  sd x16, COOP_REG_V0(TO); \
  sd x17, COOP_REG_V1(TO); \
  sd a0, COOP_REG_A0(TO); \
  sd a1, COOP_REG_A1(TO); \
  sd a2, COOP_REG_A2(TO); \
  sd a3, COOP_REG_A3(TO); \
  sd a4, COOP_REG_A4(TO); \
  sd a5, COOP_REG_A5(TO); \
  sd a6, COOP_REG_A6(TO); \
  sd a7, COOP_REG_A7(TO); \
  sd t0, COOP_REG_T0(TO); \
  sd t1, COOP_REG_T1(TO); \
  sd t2, COOP_REG_T2(TO); \
  sd t3, COOP_REG_T3(TO); \
  sd t4, COOP_REG_T4(TO); \
  sd gp, COOP_REG_GP(TO);

// Preserve the registers.  Compute the address of the trap handler.
// t0 <- %hi(trap_table)
// t1 <- mcause * ptr size
// t1 <- %hi(trap_table)[mcause]
// t1 <- trap_table[mcause]
// a0 <- regs
// a2 <- mepc
// t0 <- user sp
#define _savetostack() \
  addi sp, sp, -INTEGER_CONTEXT_SIZE; \
  STORE ra, 1*REGBYTES(sp); \
  STORE gp, 3*REGBYTES(sp); \
  STORE tp, 4*REGBYTES(sp); \
  STORE t0, 5*REGBYTES(sp); \
1:auipc t0, %pcrel_hi(trap_table);  \
  STORE t1, 6*REGBYTES(sp); \
  sll t1, a1, LOG_REGBYTES; \
  STORE t2, 7*REGBYTES(sp); \
  add t1, t0, t1; \
  STORE s0, 8*REGBYTES(sp); \
  LOAD t1, %pcrel_lo(1b)(t1); \
  STORE s1, 9*REGBYTES(sp); \
  mv a0, sp; \
  STORE a2,12*REGBYTES(sp); \
  csrr a2, mepc; \
  STORE a3,13*REGBYTES(sp); \
  csrrw t0, mscratch, x0; \
  STORE a4,14*REGBYTES(sp); \
  STORE a5,15*REGBYTES(sp); \
  STORE a6,16*REGBYTES(sp); \
  STORE a7,17*REGBYTES(sp); \
  STORE s2,18*REGBYTES(sp); \
  STORE s3,19*REGBYTES(sp); \
  STORE s4,20*REGBYTES(sp); \
  STORE s5,21*REGBYTES(sp); \
  STORE s6,22*REGBYTES(sp); \
  STORE s7,23*REGBYTES(sp); \
  STORE s8,24*REGBYTES(sp); \
  STORE s9,25*REGBYTES(sp); \
  STORE s10,26*REGBYTES(sp); \
  STORE s11,27*REGBYTES(sp); \
  STORE t3,28*REGBYTES(sp); \
  STORE t4,29*REGBYTES(sp); \
  STORE t5,30*REGBYTES(sp); \
  STORE t6,31*REGBYTES(sp); \
  STORE t0, 2*REGBYTES(sp);

// Restore all of the registers.
#define _loadfromstack() \
  LOAD ra, 1*REGBYTES(sp); \
  LOAD gp, 3*REGBYTES(sp); \
  LOAD tp, 4*REGBYTES(sp); \
  LOAD t0, 5*REGBYTES(sp); \
  LOAD t1, 6*REGBYTES(sp); \
  LOAD t2, 7*REGBYTES(sp); \
  LOAD s0, 8*REGBYTES(sp); \
  LOAD s1, 9*REGBYTES(sp); \
  LOAD a0,10*REGBYTES(sp); \
  LOAD a1,11*REGBYTES(sp); \
  LOAD a2,12*REGBYTES(sp); \
  LOAD a3,13*REGBYTES(sp); \
  LOAD a4,14*REGBYTES(sp); \
  LOAD a5,15*REGBYTES(sp); \
  LOAD a6,16*REGBYTES(sp); \
  LOAD a7,17*REGBYTES(sp); \
  LOAD s2,18*REGBYTES(sp); \
  LOAD s3,19*REGBYTES(sp); \
  LOAD s4,20*REGBYTES(sp); \
  LOAD s5,21*REGBYTES(sp); \
  LOAD s6,22*REGBYTES(sp); \
  LOAD s7,23*REGBYTES(sp); \
  LOAD s8,24*REGBYTES(sp); \
  LOAD s9,25*REGBYTES(sp); \
  LOAD s10,26*REGBYTES(sp); \
  LOAD s11,27*REGBYTES(sp); \
  LOAD t3,28*REGBYTES(sp); \
  LOAD t4,29*REGBYTES(sp); \
  LOAD t5,30*REGBYTES(sp); \
  LOAD t6,31*REGBYTES(sp); \
  LOAD sp, 2*REGBYTES(sp); \
  addi sp, sp, INTEGER_CONTEXT_SIZE;

#define _restore_context(FROM) \
  ld ra, COOP_REG_RA(FROM); \
  ld s0, COOP_REG_S0(FROM); \
  ld s1, COOP_REG_S1(FROM); \
  ld s2, COOP_REG_S2(FROM); \
  ld s3, COOP_REG_S3(FROM); \
  ld s4, COOP_REG_S4(FROM); \
  ld s5, COOP_REG_S5(FROM); \
  ld s6, COOP_REG_S6(FROM); \
  ld s7, COOP_REG_S7(FROM); \
  ld s8, COOP_REG_S8(FROM); \
  ld s9, COOP_REG_S9(FROM); \
  ld s10, COOP_REG_S10(FROM); \
  ld s11, COOP_REG_S11(FROM); \
  ld sp, COOP_REG_SP(FROM); \
  ld x16, COOP_REG_V0(FROM); \
  ld x17, COOP_REG_V1(FROM); \
  ld a0, COOP_REG_A0(FROM); \
  ld a1, COOP_REG_A1(FROM); \
  ld a2, COOP_REG_A2(FROM); \
  ld a3, COOP_REG_A3(FROM); \
  ld a4, COOP_REG_A4(FROM); \
  ld a5, COOP_REG_A5(FROM); \
  ld a6, COOP_REG_A6(FROM); \
  ld a7, COOP_REG_A7(FROM); \
  ld t0, COOP_REG_T0(FROM); \
  ld t1, COOP_REG_T1(FROM); \
  ld t2, COOP_REG_T2(FROM); \
  ld t3, COOP_REG_T3(FROM); \
  ld t4, COOP_REG_T4(FROM); \
  ld gp, COOP_REG_GP(FROM);


#define MSTATUS_MPP_M       (0x3 << 11)
#define MSTATUS_MPP_S       (0x1 << 11)
#define MSTATUS_MPRV        (1 << 17)

#define CSR_mstackovr      0xBC0
#define CSR_mstackund      0xBC1

#define SATP32_MODE 0x80000000
#define SATP32_ASID 0x7FC00000
#define SATP32_PPN  0x003FFFFF
#define SATP64_MODE 0xF000000000000000
#define SATP64_ASID 0x0FFFF00000000000
#define SATP64_PPN  0x00000FFFFFFFFFFF

#define SATP_MODE_OFF  0
#define SATP_MODE_SV32 1
#define SATP_MODE_SV39 8
#define SATP_MODE_SV48 9
#define SATP_MODE_SV57 10
#define SATP_MODE_SV64 11

// Memory Protection Unit config bits
#define PMP_R     0x01
#define PMP_W     0x02
#define PMP_X     0x04
#define PMP_A     0x18
#define PMP_L     0x80
#define PMP_SHIFT 2

#define PMP_TOR   0x08
#define PMP_NA4   0x10
#define PMP_NAPOT 0x18

/* page table entry (PTE) fields */
#define PTE_V     0x001 /* Valid */
#define PTE_R     0x002 /* Read */
#define PTE_W     0x004 /* Write */
#define PTE_X     0x008 /* Execute */
#define PTE_U     0x010 /* User */
#define PTE_G     0x020 /* Global */
#define PTE_A     0x040 /* Accessed */
#define PTE_D     0x080 /* Dirty */
#define PTE_SOFT  0x300 /* Reserved for Software */
#define PTE_RSVD  0x1FC0000000000000 /* Reserved for future standard use */
#define PTE_PBMT  0x6000000000000000 /* Svpbmt: Page-based memory types */
#define PTE_N     0x8000000000000000 /* Svnapot: NAPOT translation contiguity */
#define PTE_ATTR  0xFFC0000000000000 /* All attributes and reserved bits */

#define PTE_PPN_SHIFT 10

#define PTE_TABLE(PTE) (((PTE) & (PTE_V | PTE_R | PTE_W | PTE_X)) == PTE_V)

#if __riscv_xlen == 64
    #define MSTATUS_SD MSTATUS64_SD
    #define SSTATUS_SD SSTATUS64_SD
    #define RISCV_PGLEVEL_BITS 9
    #define SATP_MODE SATP64_MODE
    #define SATP_PPN  SATP64_PPN
#else
    #define MSTATUS_SD MSTATUS32_SD
    #define SSTATUS_SD SSTATUS32_SD
    #define RISCV_PGLEVEL_BITS 10
    #define SATP_MODE SATP32_MODE
    #define SATP_PPN  SATP32_PPN
#endif
#define RISCV_PGSHIFT 12
#define RISCV_PGSIZE (1 << RISCV_PGSHIFT)


#endif  // RISCV_CSR_ENCODING_H
