/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      RISC-V ISA specified structures and constants.
 */
#ifndef __DEBUGGER_RISCV_ISA_H__
#define __DEBUGGER_RISCV_ISA_H__

#include <inttypes.h>

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

static const uint64_t EXT_SIGN_6  = 0xFFFFFFFFFFFFFFE0LL;
static const uint64_t EXT_SIGN_8  = 0xFFFFFFFFFFFFFF00LL;
static const uint64_t EXT_SIGN_12 = 0xFFFFFFFFFFFFF000LL;
static const uint64_t EXT_SIGN_16 = 0xFFFFFFFFFFFF0000LL;
static const uint64_t EXT_SIGN_32 = 0xFFFFFFFF00000000LL;

static const char *const IREGS_NAMES[] = {
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

const char *const FREGS_NAME[] = {
  "ft0", "ft1", "ft2",  "ft3",  "ft4", "ft5", "ft6",  "ft7",
  "fs0", "fs1", "fa0",  "fa1",  "fa2", "fa3", "fa4",  "fa5",
  "fa6", "fa7", "fs2",  "fs3",  "fs4", "fs5", "fs6",  "fs7",
  "fs8", "fs9", "fs10", "fs11", "ft8", "ft9", "ft10", "ft11"
};

enum ERegNames {
    Reg_Zero,
    Reg_ra,       // [1] Return address
    Reg_sp,       // [2] Stack pointer
    Reg_gp,       // [3] Global pointer
    Reg_tp,       // [4] Thread pointer
    Reg_t0,       // [5] Temporaries 0 s3
    Reg_t1,       // [6] Temporaries 1 s4
    Reg_t2,       // [7] Temporaries 2 s5
    Reg_s0,       // [8] s0/fp Saved register/frame pointer
    Reg_s1,       // [9] Saved register 1
    Reg_a0,       // [10] Function argumentes 0
    Reg_a1,       // [11] Function argumentes 1
    Reg_a2,       // [12] Function argumentes 2
    Reg_a3,       // [13] Function argumentes 3
    Reg_a4,       // [14] Function argumentes 4
    Reg_a5,       // [15] Function argumentes 5
    Reg_a6,       // [16] Function argumentes 6
    Reg_a7,       // [17] Function argumentes 7
    Reg_s2,       // [18] Saved register 2
    Reg_s3,       // [19] Saved register 3
    Reg_s4,       // [20] Saved register 4
    Reg_s5,       // [21] Saved register 5
    Reg_s6,       // [22] Saved register 6
    Reg_s7,       // [23] Saved register 7
    Reg_s8,       // [24] Saved register 8
    Reg_s9,       // [25] Saved register 9
    Reg_s10,      // [26] Saved register 10
    Reg_s11,      // [27] Saved register 11
    Reg_t3,       // [28]
    Reg_t4,       // [29]
    Reg_t5,       // [30]
    Reg_t6,       // [31]
    Reg_Total
};


union csr_mstatus_type {
    struct bits_type {
        uint64_t UIE    : 1;    // [0]: User level interrupts ena for current
                                //      priv. mode
        uint64_t SIE    : 1;    // [1]: Super-User level interrupts ena for
                                //      current priv. mode
        uint64_t HIE    : 1;    // [2]: Hypervisor level interrupts ena for
                                //      current priv. mode
        uint64_t MIE    : 1;    // [3]: Machine level interrupts ena for
                                //      current priv. mode
        uint64_t UPIE   : 1;    // [4]: User level interrupts ena previous
                                //      value (before interrupt)
        uint64_t SPIE   : 1;    // [5]: Super-User level interrupts ena
                                //      previous value (before interrupt)
        uint64_t HPIE   : 1;    // [6]: Hypervisor level interrupts ena
                                //      previous value (before interrupt)
        uint64_t MPIE   : 1;    // [7]: Machine level interrupts ena previous
                                //      value (before interrupt)
        uint64_t SPP    : 1;    // [8]: One bit wide. Supper-user previously
                                //      priviledged level
        uint64_t HPP    : 2;    // [10:9]: the Hypervisor previous priv mode
        uint64_t MPP    : 2;    // [12:11]: the Machine previous priv mode
        uint64_t FS     : 2;    // [14:13]: RW: FPU context status
        uint64_t XS     : 2;    // [16:15]: RW: extension context status
        uint64_t MPRV   : 1;    // [17] Memory privilege bit
        uint64_t PUM    : 1;    // [18]
        uint64_t MXR    : 1;    // [19]
        uint64_t rsrv1  : 4;    // [23:20]
        uint64_t VM     : 5;    // [28:24] Virtualization management field
        uint64_t rsv2 : 64-30;  // [62:29]
        uint64_t SD     : 1;    // RO: [63] Bit summarizes FS/XS bits
    } bits;
    uint64_t value;
};

union csr_mcause_type {
    struct bits_type {
        uint64_t code   : 63;   // 11 - Machine external interrupt
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


/**
 * @name PRV bits possible values:
 */
/// @{
/// User-mode
static const uint64_t PRV_U       = 0;
/// super-visor mode
static const uint64_t PRV_S       = 1;
/// hyper-visor mode
static const uint64_t PRV_H       = 2;
//// machine mode
static const uint64_t PRV_M       = 3;
/// @}

/**
 * @name CSR registers.
 */
/// @{
/** ISA and extensions supported. */
static const uint16_t CSR_misa              = 0xf10;
/** Vendor ID. */
static const uint16_t CSR_mvendorid         = 0xf11;
/** Architecture ID. */
static const uint16_t CSR_marchid           = 0xf12;
/** Vendor ID. */
static const uint16_t CSR_mimplementationid = 0xf13;
/** Thread id (the same as core). */
static const uint16_t CSR_mhartid           = 0xf14;
/** Machine wall-clock time */
static const uint16_t CSR_mtime         = 0x701;

/** machine mode status read/write register. */
static const uint16_t CSR_mstatus       = 0x300;
/** Machine exception delegation  */
static const uint16_t CSR_medeleg       = 0x302;
/** Machine interrupt delegation  */
static const uint16_t CSR_mideleg       = 0x303;
/** Machine interrupt enable */
static const uint16_t CSR_mie           = 0x304;
/** The base address of the M-mode trap vector. */
static const uint16_t CSR_mtvec         = 0x305;
/** Machine wall-clock timer compare value. */
static const uint16_t CSR_mtimecmp      = 0x321;
/** Scratch register for machine trap handlers. */
static const uint16_t CSR_mscratch      = 0x340;
/** Exception program counters. */
static const uint16_t CSR_uepc          = 0x041;
static const uint16_t CSR_sepc          = 0x141;
static const uint16_t CSR_hepc          = 0x241;
static const uint16_t CSR_mepc          = 0x341;
/** Machine trap cause */
static const uint16_t CSR_mcause        = 0x342;
/** Machine bad address. */static const uint16_t CSR_mbadaddr      = 0x343;
/** Machine interrupt pending */
static const uint16_t CSR_mip           = 0x344;
/// @}

/** Exceptions */
enum ESignals {
    // Instruction address misaligned
    EXCEPTION_InstrMisalign,
    // Instruction access fault
    EXCEPTION_InstrFault,
    // Illegal instruction
    EXCEPTION_InstrIllegal,
    // Breakpoint
    EXCEPTION_Breakpoint,
    // Load address misaligned
    EXCEPTION_LoadMisalign,
    // Load access fault
    EXCEPTION_LoadFault,
    // Store/AMO address misaligned
    EXCEPTION_StoreMisalign,
    // Store/AMO access fault
    EXCEPTION_StoreFault,
    // Environment call from U-mode
    EXCEPTION_CallFromUmode,
    // Environment call from S-mode
    EXCEPTION_CallFromSmode,
    // Environment call from H-mode
    EXCEPTION_CallFromHmode,
    // Environment call from M-mode
    EXCEPTION_CallFromMmode,

    // User software interrupt
    INTERRUPT_USoftware,
    // Superuser software interrupt
    INTERRUPT_SSoftware,
    // Hypervisor software itnerrupt
    INTERRUPT_HSoftware,
    // Machine software interrupt
    INTERRUPT_MSoftware,
    // User timer interrupt
    INTERRUPT_UTimer,
    // Superuser timer interrupt
    INTERRUPT_STimer,
    // Hypervisor timer interrupt
    INTERRUPT_HTimer,
    // Machine timer interrupt
    INTERRUPT_MTimer,
    // User external interrupt
    INTERRUPT_UExternal,
    // Superuser external interrupt
    INTERRUPT_SExternal,
    // Hypervisor external interrupt
    INTERRUPT_HExternal,
    // Machine external interrupt (from PLIC)
    INTERRUPT_MExternal,

    SIGNAL_HardReset,
    SIGNAL_Total
};

}  // namespace debugger

#endif  // __DEBUGGER_RISCV_ISA_H__
