/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Base ISA implementation (extension I).
 */
#ifndef __DEBUGGER_RISCV_ISA_H__
#define __DEBUGGER_RISCV_ISA_H__

#include "instructions.h"

namespace debugger {

union ISA_R_type {
    struct bits_type {
        uint32_t opcode : 7;  // [6:0] 
        uint32_t rd     : 5;  // [11:7] 
        uint32_t funct3 : 3;  // [14:12] 
        uint32_t rs1    : 5;  // [19:15] 
        uint32_t rs2    : 5;  // [24:20] 
        uint32_t funct7 : 7;  // [31:25] 
    } bits;
    uint32_t value;
};

union ISA_I_type {
    struct bits_type {
        uint32_t opcode : 7;  // [6:0] 
        uint32_t rd     : 5;  // [11:7] 
        uint32_t funct3 : 3;  // [14:12] 
        uint32_t rs1    : 5;  // [19:15] 
        uint32_t imm    : 12;  // [31:20] 
    } bits;
    uint32_t value;
};

union ISA_S_type {
    struct bits_type {
        uint32_t opcode : 7;  // [6:0] 
        uint32_t imm4_0 : 5;  // [11:7] 
        uint32_t funct3 : 3;  // [14:12] 
        uint32_t rs1    : 5;  // [19:15] 
        uint32_t rs2    : 5;  // [24:20] 
        uint32_t imm11_5 : 7;  // [31:25] 
    } bits;
    uint32_t value;
};

union ISA_SB_type {
    struct bits_type {
        uint32_t opcode : 7;  // [6:0] 
        uint32_t imm11  : 1;  // [7] 
        uint32_t imm4_1 : 4;  // [11:8] 
        uint32_t funct3 : 3;  // [14:12] 
        uint32_t rs1    : 5;  // [19:15] 
        uint32_t rs2    : 5;  // [24:20] 
        uint32_t imm10_5 : 6;  // [30:25] 
        uint32_t imm12   : 1;  // [31] 
    } bits;
    uint32_t value;
};

union ISA_U_type {
    struct bits_type {
        uint32_t opcode : 7;  // [6:0] 
        uint32_t rd     : 5;  // [11:7] 
        uint32_t imm31_12 : 20;  // [31:12] 
    } bits;
    uint32_t value;
};

union ISA_UJ_type {
    struct bits_type {
        uint32_t opcode   : 7;   // [6:0] 
        uint32_t rd       : 5;   // [11:7] 
        uint32_t imm19_12 : 8;   // [19:12] 
        uint32_t imm11    : 1;   // [20] 
        uint32_t imm10_1  : 10;  // [30:21] 
        uint32_t imm20    : 1;   // [31] 
    } bits;
    uint32_t value;
};

static const uint64_t EXT_SIGN_8  = 0xFFFFFFFFFFFFFF00LL;
static const uint64_t EXT_SIGN_12 = 0xFFFFFFFFFFFFF000LL;
static const uint64_t EXT_SIGN_16 = 0xFFFFFFFFFFFF0000LL;
static const uint64_t EXT_SIGN_32 = 0xFFFFFFFF00000000LL;

static const uint64_t Except_Illegal_Instruction = 1;

static const char *const REG_NAMES[32] = {
    "zero",     // [0] zero
    "ra",       // [1] Return address
    "sp",       // [2] Stack pointer
    "gp",       // [3] Global pointer
    "tp",       // [4] Thread pointer
    "t0",       // [5] Temporaries 0 s3
    "t1",       // [6] Temporaries 1 s4
    "t2",       // [7] Temporaries 2 s5
    "s0",       // [8] s0/fp Saved register/frame pointer
    "s1",       // [9] Saved register 1
    "a0",       // [10] Function argumentes 0
    "a1",       // [11] Function argumentes 1
    "a2",       // [12] Function argumentes 2
    "a3",       // [13] Function argumentes 3
    "a4",       // [14] Function argumentes 4
    "a5",       // [15] Function argumentes 5
    "a6",       // [16] Function argumentes 6
    "a7",       // [17] Function argumentes 7
    "s2",       // [18] Saved register 2
    "s3",       // [19] Saved register 3
    "s4",       // [20] Saved register 4
    "s5",       // [21] Saved register 5
    "s6",       // [22] Saved register 6
    "s7",       // [23] Saved register 7
    "s8",       // [24] Saved register 8
    "s9",       // [25] Saved register 9
    "s10",      // [26] Saved register 10
    "s11",      // [27] Saved register 11
    "t3",       // [28] 
    "t4",       // [29] 
    "t5",       // [30] 
    "t6"        // [31] 
};

union csr_mstatus_type {
    struct bits_type {
        uint64_t IE     : 1;    // interrupts ena for current priv. mode
        uint64_t PRV    : 2;    // current priv mode value
        uint64_t IE1    : 1;
        uint64_t PRV1   : 2;
        uint64_t IE2    : 1;
        uint64_t PRV2   : 2;
        uint64_t IE3    : 1;
        uint64_t PRV3   : 2;
        uint64_t FS     : 2;    // RW: FPU context status
        uint64_t XS     : 2;    // RW: extension context status
        uint64_t MPRV   : 1;    // [16] Memory privilege bit
        uint64_t VM     : 5;    // [21:17] Virtualization management field
        uint64_t rsrv   : 41;
        uint64_t SD     : 1;    // RO: [63] Bit summarizes FS/XS bits
    } bits;
    uint64_t value;
};

union csr_mcause_type {
    struct bits_type {
        uint64_t code   : 4;
        uint64_t rsrv   : 59;
        uint64_t irq    : 1;
    } bits;
    uint64_t value;
};

union csr_mie_type {
    struct bits_type {
        uint64_t zero1  : 1;
        uint64_t SSIE   : 1;    // super-visor software interrupt enable
        uint64_t HSIE   : 1;    // hyper-visor software interrupt enable
        uint64_t MSIE   : 1;    // machine mode software interrupt enable
        uint64_t zero2  : 1;
        uint64_t STIE   : 1;    // super-visor time interrupt enable
        uint64_t HTIE   : 1;    // hyper-visor time interrupt enable
        uint64_t MTIE   : 1;    // machine mode time interrupt enable
    } bits;
    uint64_t value;
};

union csr_mip_type {
    struct bits_type {
        uint64_t zero1  : 1;
        uint64_t SSIP   : 1;    // super-visor software interrupt pending
        uint64_t HSIP   : 1;    // hyper-visor software interrupt pending
        uint64_t MSIP   : 1;    // machine mode software interrupt pending
        uint64_t zero2  : 1;
        uint64_t STIP   : 1;    // super-visor time interrupt pending
        uint64_t HTIP   : 1;    // hyper-visor time interrupt pending
        uint64_t MTIP   : 1;    // machine mode time interrupt pending
    } bits;
    uint64_t value;
};

static const uint64_t RESET_VECTOR      = 0x200;

/**
 * @name PRV bits possible values:
 */
/// @{
/// User-mode
static const uint64_t PRV_LEVEL_U       = 0;
/// super-visor mode
static const uint64_t PRV_LEVEL_S       = 1;
/// hyper-visor mode
static const uint64_t PRV_LEVEL_H       = 2;
//// machine mode
static const uint64_t PRV_LEVEL_M       = 3;
/// @}

/**
 * @name CSR registers.
 */
/// @{
/** CPU description */
static const uint16_t CSR_mcpuid        = 0xf00;
/**
 * Vendor ID and revision number (See table 3.2)
 *
 * Bits[15:0] Source
 *      0000            CPU ID unimplemented *      0001            UC Berkeley Rocket repo
 *      0x0002–0x7FFE   Reserved for open-source repos *      ...             
 */
static const uint16_t CSR_mimpid        = 0xf01;
/**
 * @brief Thread id (the same as core).
 *
 * The mhartid register is an XLEN-bit read-only register containing 
 * the integer ID of the hardware thread running the code. This register must 
 * be readable in any implementation. Hart IDs might not necessarily be 
 * numbered contiguously in a multiprocessor system, but at least one hart must
 * have a hart ID of zero. */
static const uint16_t CSR_mheartid      = 0xf10;
/** machine mode status read/write register. */
static const uint16_t CSR_mstatus       = 0x300;
/** Machine wall-clock time */
static const uint16_t CSR_mtime         = 0x701;
/** Software reset. */
static const uint16_t CSR_mreset        = 0x782;
/** Inter-processor interrupt */
static const uint16_t CSR_send_ipi      = 0x783;

/**
 * @brief The base address of the M-mode trap vector.
 *
 * Low Trap Vector Addresses:
 *      0x100 Trap from user-mode
 *      0x140 Trap from supervisor-mode
 *      0x180 Trap from hypervisor-mode
 *      0x1C0 Trap from machine-mode
 *      0x1FC Non-maskable interrupt(s)
 *      0x200 Reset vector
 */
static const uint16_t CSR_mtvec         = 0x301;
/** Machine interrupt enable */
static const uint16_t CSR_mie           = 0x304;
/** Scratch register for machine trap handlers. */
static const uint16_t CSR_mscratch      = 0x340;
/** Machine exception program counter. */
static const uint16_t CSR_mepc          = 0x341;
/** Machine trap cause */
static const uint16_t CSR_mcause        = 0x342;
/** Machine bad address. */static const uint16_t CSR_mbadaddr      = 0x343;
/** Machine interrupt pending */
static const uint16_t CSR_mip           = 0x344;
/// @}

/** Exceptions */

// Instruction address misaligned
static const uint64_t EXCEPTION_InstrMisalign   = 0;
// Instruction access fault
static const uint64_t EXCEPTION_InstrFault      = 1;
// Illegal instruction
static const uint64_t EXCEPTION_InstrIllegal    = 2;
// Breakpoint
static const uint64_t EXCEPTION_Breakpoint      = 3;
// Load address misaligned
static const uint64_t EXCEPTION_LoadMisalign    = 4;
// Load access fault
static const uint64_t EXCEPTION_LoadFault       = 5;
//Store/AMO address misaligned
static const uint64_t EXCEPTION_StoreMisalign   = 6;
// Store/AMO access fault
static const uint64_t EXCEPTION_StoreFault      = 7;
//Environment call from U-mode
static const uint64_t EXCEPTION_CallFromUmode   = 8;
// Environment call from S-mode
static const uint64_t EXCEPTION_CallFromSmode   = 9;
// Environment call from H-mode
static const uint64_t EXCEPTION_CallFromHmode   = 10;
// Environment call from M-mode
static const uint64_t EXCEPTION_CallFromMmode   = 11;

/** Interrupts: */
// Software interrupt
static const uint64_t IRQ_Software = 0; 
// Timer interrupt
static const uint64_t IRQ_Timer = 1;

void addIsaUserRV64I(CpuDataType *data, AttributeType *out);
void addIsaPrivilegedRV64I(CpuDataType *data, AttributeType *out);
void addIsaExtensionA(CpuDataType *data, AttributeType *out);
void addIsaExtensionF(CpuDataType *data, AttributeType *out);
void addIsaExtensionM(CpuDataType *data, AttributeType *out);


void generateInterrupt(uint64_t code, CpuDataType *data);
void generateException(uint64_t code, CpuDataType *data);

uint64_t readCSR(uint32_t idx, CpuDataType *data);
void writeCSR(uint32_t idx, uint64_t val, CpuDataType *data);

}  // namespace debugger

#endif  // __DEBUGGER_RISCV_ISA_H__
