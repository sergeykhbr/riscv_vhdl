/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef __DEBUGGER_RISCV_ISA_H__
#define __DEBUGGER_RISCV_ISA_H__

#include <inttypes.h>
#include <generic-isa.h>

namespace debugger {

#define UPDT2

union ISA_R_type {
    struct bits_type {
        uint32_t opcode : 7;  // [6:0]
        uint32_t rd     : 5;  // [11:7]
        uint32_t funct3 : 3;  // [14:12]
        uint32_t rs1    : 5;  // [19:15]
        uint32_t rs2    : 5;  // [24:20]
        uint32_t funct7 : 7;  // [31:25]
    } bits;
    // atomic
    struct amo_bits_type {
        uint32_t opcode : 7;  // [6:0]
        uint32_t rd     : 5;  // [11:7]
        uint32_t funct3 : 3;  // [14:12]
        uint32_t rs1    : 5;  // [19:15]
        uint32_t rs2    : 5;  // [24:20]
        uint32_t rl     : 1;  // [25]
        uint32_t aq     : 1;  // [26]
        uint32_t funct5 : 5;  // [31:27]
    } amobits;
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

/**
 * Compressed extension types:
 */

// Regsiter
union ISA_CR_type {
    struct bits_type {
        uint16_t opcode : 2;  // [1:0]
        uint16_t rs2    : 5;  // [6:2]
        uint16_t rdrs1  : 5;  // [11:7]
        uint16_t funct4 : 4;  // [15:12]
    } bits;
    uint16_t value;
};

// Immediate
union ISA_CI_type {
    struct bits_type {
        uint16_t opcode : 2;  // [1:0]
        uint16_t imm    : 5;  // [6:2]
        uint16_t rdrs   : 5;  // [11:7]
        uint16_t imm6   : 1;  // [12]
        uint16_t funct3 : 3;  // [15:13]
    } bits;
    struct sp_bits_type {
        uint16_t opcode : 2;  // [1:0]
        uint16_t imm5    : 1; // [2]
        uint16_t imm8_7  : 2; // [4:3]
        uint16_t imm6  : 1;   // [5]
        uint16_t imm4  : 1;   // [6]
        uint16_t sp    : 5;   // [11:7]
        uint16_t imm9   : 1;  // [12]
        uint16_t funct3 : 3;  // [15:13]
    } spbits;
    struct ldsp_bits_type {
        uint16_t opcode : 2;  // [1:0]
        uint16_t off8_6 : 3;  // [4:2]
        uint16_t off4_3 : 2;  // [6:5]
        uint16_t rd     : 5;  // [11:7]
        uint16_t off5   : 1;  // [12]
        uint16_t funct3 : 3;  // [15:13]
    } ldspbits;
    struct lwsp_bits_type {
        uint16_t opcode : 2;  // [1:0]
        uint16_t off7_6 : 2;  // [3:2]
        uint16_t off4_2 : 3;  // [6:4]
        uint16_t rd     : 5;  // [11:7]
        uint16_t off5   : 1;  // [12]
        uint16_t funct3 : 3;  // [15:13]
    } lwspbits;
    uint16_t value;
};

// Stack relative Store
union ISA_CSS_type {
    struct w_bits_type {
        uint16_t opcode : 2;  // [1:0]
        uint16_t rs2    : 5;  // [6:2]
        uint16_t imm7_6 : 2;  // [8:7]
        uint16_t imm5_2 : 4;  // [12:9]
        uint16_t funct3 : 3;  // [15:13]
    } wbits;
    struct d_bits_type {
        uint16_t opcode : 2;  // [1:0]
        uint16_t rs2    : 5;  // [6:2]
        uint16_t imm8_6 : 3;  // [9:7]
        uint16_t imm5_3 : 3;  // [12:10]
        uint16_t funct3 : 3;  // [15:13]
    } dbits;
    uint16_t value;
};

// Wide immediate
union ISA_CIW_type {
    struct bits_type {
        uint16_t opcode : 2;  // [1:0]
        uint16_t rd     : 3;  // [4:2]
        uint16_t imm3   : 1;  // [5]
        uint16_t imm2   : 1;  // [6]
        uint16_t imm9_6 : 4;  // [10:7]
        uint16_t imm5_4 : 2;  // [12:11]
        uint16_t funct3 : 3;  // [15:13]
    } bits;
    uint16_t value;
};

// Load
union ISA_CL_type {
    struct bits_type {
        uint16_t opcode : 2;  // [1:0]
        uint16_t rd     : 3;  // [4:2]
        uint16_t imm6   : 1;  // [5]
        uint16_t imm27  : 1;  // [6]
        uint16_t rs1    : 3;  // [9:7]
        uint16_t imm5_3 : 3;  // [12:10]
        uint16_t funct3 : 3;  // [15:13]
    } bits;
    uint16_t value;
};

// Store
union ISA_CS_type {
    struct bits_type {
        uint16_t opcode : 2;  // [1:0]
        uint16_t rs2    : 3;  // [4:2]
        uint16_t imm6   : 1;  // [5]
        uint16_t imm27  : 1;  // [6]
        uint16_t rs1    : 3;  // [9:7]
        uint16_t imm5_3 : 3;  // [12:10]
        uint16_t funct3 : 3;  // [15:13]
    } bits;
    uint16_t value;
};

// Branch
union ISA_CB_type {
    struct bits_type {
        uint16_t opcode : 2;  // [1:0]
        uint16_t off5   : 1;  // [2]
        uint16_t off2_1 : 2;  // [4:3]
        uint16_t off7_6 : 2;  // [6:5]
        uint16_t rs1    : 3;  // [9:7]
        uint16_t off4_3 : 2;  // [11:10]
        uint16_t off8   : 1;  // [12]
        uint16_t funct3 : 3;  // [15:13]
    } bits;
    struct sh_bits_type {
        uint16_t opcode : 2;  // [1:0]
        uint16_t shamt  : 5;  // [6:2]
        uint16_t rd     : 3;  // [9:7]
        uint16_t funct2 : 2;  // [11:10]
        uint16_t shamt5 : 1;  // [12]
        uint16_t funct3 : 3;  // [15:13]
    } shbits;
    uint16_t value;
};

// Jump
union ISA_CJ_type {
    struct bits_type {
        uint16_t opcode : 2;  // [1:0]
        uint16_t off5   : 1;  // [2]
        uint16_t off3_1 : 3;  // [5:3]
        uint16_t off7   : 1;  // [6]
        uint16_t off6   : 1;  // [7]
        uint16_t off10  : 1;  // [8]
        uint16_t off9_8 : 2;  // [10:9]
        uint16_t off4   : 1;  // [11]
        uint16_t off11  : 1;  // [12]
        uint16_t funct3 : 3;  // [15:13]
    } bits;
    uint16_t value;
};

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
    "t6",       // [31]
    // RegFpu_Offset
    "ft0", "ft1", "ft2",  "ft3",  "ft4", "ft5", "ft6",  "ft7",
    "fs0", "fs1", "fa0",  "fa1",  "fa2", "fa3", "fa4",  "fa5",
    "fa6", "fa7", "fs2",  "fs3",  "fs4", "fs5", "fs6",  "fs7",
    "fs8", "fs9", "fs10", "fs11", "ft8", "ft9", "ft10", "ft11"
};

static const ECpuRegMapping RISCV_DEBUG_REG_MAP[] = {
    {"npc",   8, CSR_dpc},
    {"steps", 8, CSR_insret},
    {"zero",  8, 0x1000},
    {"ra",    8, 0x1001},
    {"sp",    8, 0x1002},
    {"gp",    8, 0x1003},
    {"tp",    8, 0x1004},
    {"t0",    8, 0x1005},
    {"t1",    8, 0x1006},
    {"t2",    8, 0x1007},
    {"s0",    8, 0x1008},
    {"s1",    8, 0x1009},
    {"a0",    8, 0x100A},
    {"a1",    8, 0x100B},
    {"a2",    8, 0x100C},
    {"a3",    8, 0x100D},
    {"a4",    8, 0x100E},
    {"a5",    8, 0x100F},
    {"a6",    8, 0x1010},
    {"a7",    8, 0x1011},
    {"s2",    8, 0x1012},
    {"s3",    8, 0x1013},
    {"s4",    8, 0x1014},
    {"s5",    8, 0x1015},
    {"s6",    8, 0x1016},
    {"s7",    8, 0x1017},
    {"s8",    8, 0x1018},
    {"s9",    8, 0x1019},
    {"s10",   8, 0x101A},
    {"s11",   8, 0x101B},
    {"t3",    8, 0x101C},
    {"t4",    8, 0x101D},
    {"t5",    8, 0x101E},
    {"t6",    8, 0x101F},
    {"ft0",   8, 0x1020},
    {"ft1",   8, 0x1021},
    {"ft2",   8, 0x1022},
    {"ft3",   8, 0x1023},
    {"ft4",   8, 0x1024},
    {"ft5",   8, 0x1025},
    {"ft6",   8, 0x1026},
    {"ft7",   8, 0x1027},
    {"fs0",   8, 0x1028},
    {"fs1",   8, 0x1029},
    {"fa0",   8, 0x102A},
    {"fa1",   8, 0x102B},
    {"fa2",   8, 0x102C},
    {"fa3",   8, 0x102D},
    {"fa4",   8, 0x102E},
    {"fa5",   8, 0x102F},
    {"fa6",   8, 0x1030},
    {"fa7",   8, 0x1031},
    {"fs2",   8, 0x1032},
    {"fs3",   8, 0x1033},
    {"fs4",   8, 0x1034},
    {"fs5",   8, 0x1035},
    {"fs6",   8, 0x1036},
    {"fs7",   8, 0x1037},
    {"fs8",   8, 0x1038},
    {"fs9",   8, 0x1039},
    {"fs10",  8, 0x103A},
    {"fs11",  8, 0x103B},
    {"ft8",   8, 0x103C},
    {"ft9",   8, 0x103D},
    {"ft10",  8, 0x103E},
    {"ft11",  8, 0x103F},
    {"",      0, 0}
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

static const int RegFpu_Offset = Reg_Total;

enum ERegFpuNames {
    Reg_f0,     // ft0 temporary register
    Reg_f1,     // ft1
    Reg_f2,     // ft2
    Reg_f3,     // ft3
    Reg_f4,     // ft4
    Reg_f5,     // ft5
    Reg_f6,     // ft6
    Reg_f7,     // ft7
    Reg_f8,     // fs0 saved register
    Reg_f9,     // fs1
    Reg_f10,    // fa0 argument/return value
    Reg_f11,    // fa1 argument/return value
    Reg_f12,    // fa2 argument register
    Reg_f13,    // fa3
    Reg_f14,    // fa4
    Reg_f15,    // fa5
    Reg_f16,    // fa6
    Reg_f17,    // fa7
    Reg_f18,    // fs2 saved register
    Reg_f19,    // fs3
    Reg_f20,    // fs4
    Reg_f21,    // fs5
    Reg_f22,    // fs6
    Reg_f23,    // fs7
    Reg_f24,    // fs8
    Reg_f25,    // fs9
    Reg_f26,    // fs10
    Reg_f27,    // fs11
    Reg_f28,    // ft8 temporary register
    Reg_f29,    // ft9
    Reg_f30,    // ft10
    Reg_f31,    // ft11
    RegFpu_Total
};

static const int REGS_BUS_WIDTH = 6;
static const int REGS_TOTAL = (1 << REGS_BUS_WIDTH);//Reg_Total + RegFpu_Total;

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

union csr_fcsr_type {
    struct bits_type {
        uint64_t NX : 1;        // Inexact
        uint64_t UF : 1;        // Underflow
        uint64_t OF : 1;        // Overflow
        uint64_t DZ : 1;        // Divide by Zero
        uint64_t NV : 1;        // Invalid operation
        uint64_t FRM : 3;       // rounding mode
        uint64_t rsrv1 : 56;
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

enum EExceptions {
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
    // Instruction page fault
    EXCEPTION_InstrPageFault,
    // Load page fault
    EXCEPTION_LoadPageFault,
    // reserved
    EXCEPTION_rsrv14,
    // Store/AMO page fault
    EXCEPTION_StorePageFault,
    // Stack overflow
    EXCEPTION_StackOverflow,
    // Stack underflow
    EXCEPTION_StackUnderflow,
    EXCEPTIONS_Total
};

enum EInterrupts {
    INTERRUPT_XSoftware,
    INTERRUPT_XTimer,
    INTERRUPT_XExternal,
    INTERRUPT_Total
};
/** Exceptions */
enum ESignals {
    SIGNAL_Exception = 0,
    SIGNAL_XSoftware = EXCEPTIONS_Total + 4*INTERRUPT_XSoftware,
    SIGNAL_XTimer = EXCEPTIONS_Total + 4*INTERRUPT_XTimer,
    SIGNAL_XExternal = EXCEPTIONS_Total + 4*INTERRUPT_XExternal,
    SIGNAL_HardReset,
    SIGNAL_Total
};

static const int EXCEPTION_CallFromXMode    = EXCEPTION_CallFromUmode;

}  // namespace debugger

#endif  // __DEBUGGER_RISCV_ISA_H__
