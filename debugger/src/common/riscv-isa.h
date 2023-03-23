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
#include <api_types.h>

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

union csr_mstatus_type {
    struct bits_type {
        uint64_t rsrv0  : 1;    // [0]: 
        uint64_t SIE    : 1;    // [1]: Super-User level interrupts ena for
                                //      current priv. mode
        uint64_t rsrv2  : 1;    // [2]
        uint64_t MIE    : 1;    // [3]: Machine level interrupts ena for
                                //      current priv. mode
        uint64_t rsrv4  : 1;    // [4]: 
        uint64_t SPIE   : 1;    // [5]: Super-User level interrupts ena
                                //      previous value (before interrupt)
        uint64_t UBE    : 1;    // [6]: 
        uint64_t MPIE   : 1;    // [7]: Machine level interrupts ena previous
                                //      value (before interrupt)
        uint64_t SPP    : 1;    // [8]: One bit wide. Supper-user previously
                                //      priviledged level
        uint64_t VS     : 2;    // [10:9]: the Hypervisor previous priv mode
        uint64_t MPP    : 2;    // [12:11]: WARL Machine previous priv mode
        uint64_t FS     : 2;    // [14:13]: RW: FPU context status
        uint64_t XS     : 2;    // [16:15]: RW: extension context status
        uint64_t MPRV   : 1;    // [17] Memory privilege bit
        uint64_t SUM    : 1;    // [18]
        uint64_t MXR    : 1;    // [19]
        uint64_t TVM    : 1;    // [20]
        uint64_t TW     : 1;    // [21]
        uint64_t TSR    : 1;    // [22]
        uint64_t rsrv31_23 : 9; // [31:23]
        uint64_t UXL    : 2;    // [33:32]
        uint64_t SXL    : 2;    // [35:34]
        uint64_t SBE    : 1;    // [36]
        uint64_t MBE    : 1;    // [37]
        uint64_t rsv62_38 : 25; // [62:38]
        uint64_t SD     : 1;    // RO: [63] Bit summarizes FS/XS bits
    } bits;
    uint64_t value;
};

union csr_mcause_type {
    struct bits_type {
        uint64_t code   : 63;   // 11 - Machine external interrupt; 9 - Supervisor external interrupt
        uint64_t irq    : 1;
    } bits;
    uint64_t value;
};

union csr_mie_type {
    struct bits_type {
        uint64_t USIE   : 1;    // [0] Use sw interrupt
        uint64_t SSIE   : 1;    // [1] super-visor software interrupt enable
        uint64_t HSIE   : 1;    // [2] hyper-visor software interrupt enable
        uint64_t MSIE   : 1;    // [3] machine mode software interrupt enable
        uint64_t UTIE   : 1;    // [4]
        uint64_t STIE   : 1;    // [5] super-visor time interrupt enable
        uint64_t HTIE   : 1;    // [6] hyper-visor time interrupt enable
        uint64_t MTIE   : 1;    // [7] machine mode time interrupt enable
        uint64_t UEIE   : 1;    // [8] User external interrupt enable
        uint64_t SEIE   : 1;    // [9] supervisor external interrupt enable
        uint64_t HEIE   : 1;    // [10] hypervisor external interrupt enable
        uint64_t MEIE   : 1;    // [11] machine external interrupt enable
    } bits;
    uint64_t value;
};

union csr_mip_type {
    struct bits_type {
        uint64_t USIP   : 1;
        uint64_t SSIP   : 1;    // super-visor software interrupt pending
        uint64_t HSIP   : 1;    // hyper-visor software interrupt pending
        uint64_t MSIP   : 1;    // machine mode software interrupt pending
        uint64_t UTIP   : 1;
        uint64_t STIP   : 1;    // super-visor time interrupt pending
        uint64_t HTIP   : 1;    // hyper-visor time interrupt pending
        uint64_t MTIP   : 1;    // machine mode time interrupt pending
        uint64_t UEIP   : 1;    // [8] User external interrupt pending
        uint64_t SEIP   : 1;    // [9] supervisor external interrupt pending
        uint64_t HEIP   : 1;    // [10] hypervisor external interrupt pending
        uint64_t MEIP   : 1;    // [11] machine external interrupt pending
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

// Debug Control and Status (dcsr, at 0x7b0)
union csr_dcsr_type {
    uint64_t u64;
    uint32_t u32[2];
    struct bits_type {
        uint64_t prv : 2;       // [1:0] Operational mode when Debug mode was entered
        uint64_t step : 1;      // [2] RW. Execute a single instruction
        uint64_t nmip : 1;      // [3] R. NMI pending bit for the hart
        uint64_t mprven : 1;    // [4] WARL. 0(disabled) MPRV in mstatus is ignored in Debug mode. 1(enabled)
        uint64_t v : 1;         // [5] WARL. Extends the prv mode. 0 when virtualization is not supported
        uint64_t cause : 3;     // [8:6] R. 1=ebreak; 2=trigger; 3=haltreq; 4=step; 5=resethaltreq; 6=group, was halted because it is part of the group
        uint64_t stoptime : 1;  // [9] WARL. 0(normal)=time continues to reflect mtime; 1(freez)=time is frozen at the Debug mode
        uint64_t stopcount : 1; // [10] WARL. 0(normal)=increment counters as usual; 1(freez)=don't increment any hart-local counters
        uint64_t stepie : 1;    // [11] WARL. 0=interrupts disabled (including NMI); 1=interrupts enabled
        uint64_t ebreaku : 1;   // [12] WARL. 0(exception): ebreak instruction in U-mode behave as in Priv spec. 1(debug mode): ebreak instr. in U-mode enter Debug Mode
        uint64_t ebreaks : 1;   // [13] WARL. 0(exception): ebreak instruction in S-mode behave as in Priv spec. 1(debug mode): ebreak instr. in S-mode enter Debug Mode
        uint64_t rsrv14 : 1;    // [14]
        uint64_t ebreakm : 1;   // [15] RWL. 0(exception): ebreak instruction in M-mode behave as in Priv spec. 1(debug mode): ebreak instr. in M-mode enter Debug Mode
        uint64_t ebreakvu : 1;  // [16] WARL. 0(exception): ebreak instruction in VU-mode behave as in Priv spec. 1(debug mode): ebreak instr. in VU-mode enter Debug Mode
        uint64_t ebreakvs : 1;  // [17] WARL. 0(exception): ebreak instruction in VS-mode behave as in Priv spec. 1(debug mode): ebreak instr. in VS-mode enter Debug Mode
        uint64_t rsrv27_18 : 10;// [27:18]
        uint64_t debugver : 4;  // [31:28] R. 0=no debug support; 4=(1.0)
    } bits;
};

// Trigger Data1
union TriggerData1Type {
    uint64_t val;
    uint8_t u8[8];
    struct bits_type {
        uint64_t data : 59;     // [58:0]
        uint64_t dmode : 1;     // [59]
        uint64_t type : 4;      // [63:60]
    } bitsdef;
    struct bits_type2 {
        uint64_t load : 1;      // [0]
        uint64_t store : 1;     // [1]
        uint64_t execute : 1;   // [2]
        uint64_t u : 1;         // [3]
        uint64_t s : 1;         // [4]
        uint64_t rsr5 : 1;      // [5]
        uint64_t m : 1;         // [6]
        uint64_t match : 4;     // [10:7]
        uint64_t chain : 1;     // [11]
        uint64_t action : 4;    // [15:12]
        uint64_t sizelo : 2;    // [17:16]
        uint64_t timing : 1;    // [18]
        uint64_t select : 1;    // [19]
        uint64_t hit : 1;       // [20]
        uint64_t sizehi : 2;    // [22:21]
        uint64_t rsrv_23 : 30;  // [52:23]
        uint64_t maskmax : 6;   // [58:53]
        uint64_t dmode : 1;     // [59]
        uint64_t type : 4;      // [63:60]
    } mcontrol_bits;
    struct bits_type3 {
        uint64_t action : 6;    // [5:0]: 0=raise breakpoint exception; 1=Enter Debug Mode
        uint64_t u : 1;         // [6]
        uint64_t s : 1;         // [7]
        uint64_t rsr5 : 1;      // [8]
        uint64_t m : 1;         // [9]
        uint64_t count : 14;    // [23:10]
        uint64_t hit : 1;       // [24]
        uint64_t rsrv57_10 : 34;// [58:25]
        uint64_t dmode : 1;     // [59]
        uint64_t type : 4;      // [63:60]
    } icount_bits;
    struct bits_type4 {         // the same for type4 and type5
        uint64_t action : 6;    // [5:0]: 0=raise breakpoint exception; 1=Enter Debug Mode
        uint64_t u : 1;         // [6]
        uint64_t s : 1;         // [7]
        uint64_t rsr5 : 1;      // [8]
        uint64_t m : 1;         // [9]
        uint64_t rsrv57_10 : 48;// [57:10]
        uint64_t hit : 1;       // [58]
        uint64_t dmode : 1;     // [59]
        uint64_t type : 4;      // [63:60]
    } itrigger_bits;
};

static const uint64_t SATP_MODE_OFF  = 0ull;
static const uint64_t SATP_MODE_SV32 = 1ull;
static const uint64_t SATP_MODE_SV39 = 8ull;
static const uint64_t SATP_MODE_SV48 = 9ull;
static const uint64_t SATP_MODE_SV57 = 10ull;
static const uint64_t SATP_MODE_SV64 = 11ull;

union csr_satp_type {
    uint64_t u64;
    struct bits_type {
        uint64_t ppn : 44;  // [43:0] WARL
        uint64_t asid : 9;  // [59:44] WARL
        uint64_t mode : 4;  // [63:60] WARL
    } bits;
};


static const char *const RISCV_IREGS_NAMES[] = {
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
    {"pc",   8, 0x7b1}, //CSR_dpc},
    {"insret", 8, 0xC02}, //CSR_insret},
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


}  // namespace debugger

#endif  // __DEBUGGER_RISCV_ISA_H__
