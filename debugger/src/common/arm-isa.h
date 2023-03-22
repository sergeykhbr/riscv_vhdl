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

#ifndef __DEBUGGER_COMMON_ARM_ISA_H__
#define __DEBUGGER_COMMON_ARM_ISA_H__

#include <inttypes.h>
#include <api_types.h>

namespace debugger {

/** opcodes:
    0000 = AND - Rd:= Op1 AND Op2
    0001 = EOR - Rd:= Op1 EOR Op2
    0010 = SUB - Rd:= Op1 - Op2
    0011 = RSB - Rd:= Op2 - Op1
    0100 = ADD - Rd:= Op1 + Op2
    0101 = ADC - Rd:= Op1 + Op2 + C
    0110 = SBC - Rd:= Op1 - Op2 + C
    0111 = RSC - Rd:= Op2 - Op1 + C
    1000 = TST - set condition codes on Op1 AND Op2
    1001 = TEQ - set condition codes on Op1 EOR Op2
    1010 = CMP - set condition codes on Op1 - Op2
    1011 = CMN - set condition codes on Op1 + Op2
    1100 = ORR - Rd:= Op1 OR Op2
    1101 = MOV - Rd:= Op2
    1110 = BIC - Rd:= Op1 AND NOT Op2
    1111 = MVN - Rd:= NOT Op2
*/
union DataProcessingType {
    struct reg_bits_type {
        uint32_t rm : 4;        // [3:0] 2-nd operand register
        uint32_t sh_sel : 1;    // [4] 0=shift amount in [11:7], 1=Rs in [11:8]
        uint32_t sh_type : 2;   // [6:5] 0=logic left; 1=logic right;
                                //       2=arith right; 3=rotate right
        uint32_t shift : 5;     // [11:7] shift applied to Rm
        uint32_t rd : 4;        // [15:12]
        uint32_t rn : 4;        // [19:16] 1-st operand register
        uint32_t S : 1;         // [20]. 0=do not alter condition code
        uint32_t opcode : 4;    // [24:21]
        uint32_t I : 1;         // [25] = 0 for register instruction
        uint32_t zero : 2;      // [27:26] = 00b
        uint32_t cond : 4;      // [31:28]
    } reg_bits;
    struct imm_bits_type {
        uint32_t imm : 8;       // [7:0]
        uint32_t rotate : 4;    // [11:8] rotate applied to imm
        uint32_t rd : 4;        // [15:12]
        uint32_t rn : 4;        // [19:16]
        uint32_t S : 1;         // [20]. 0=do not alter condition code
        uint32_t opcode : 4;    // [24:21]
        uint32_t I : 1;         // [25] = 1 for immediate instruction
        uint32_t zero : 2;      // [27:26] = 00b
        uint32_t cond : 4;      // [31:28]
    } imm_bits;
    struct mrs_bits_type {
        uint32_t zero12 : 12;   // [11:0] = 0
        uint32_t rd : 4;        // [15:12] destination
        uint32_t mask : 4;      // [21:16] 
        uint32_t b21_20 : 2;    // [21:20] 00b=MRS; 10=MSR
        uint32_t ps : 1;        // [22] 0=CPSR; 1=SPSR_<cur_mod>
        uint32_t b27_23 : 5;    // [27:23] contant 00010b
        uint32_t cond : 4;      // [31:28]
    } mrs_bits;
    struct mov_bits_type {
        uint32_t imm12 : 12;
        uint32_t rd : 4;
        uint32_t imm4 : 4;
        uint32_t b27_20 : 8;
        uint32_t cond : 4;
    } mov_bits;
    uint32_t value;
};

union SingleDataTransferType {
    struct reg_bits_type {
        uint32_t rm : 4;        // [3:0] offset register
        uint32_t sh_sel : 8;    // [11:4] shift applied to Rm
        uint32_t rd : 4;        // [15:12]
        uint32_t rn : 4;        // [19:16]
        uint32_t L : 1;         // [20] = 1 load; 0 store
        uint32_t W : 1;         // [21] = 1 wr addr into base; 0 no write-back
        uint32_t B : 1;         // [22] = 1 byte; 0 word
        uint32_t U : 1;         // [23] = 1 add offset; 0 subtruct offset
        uint32_t P : 1;         // [24] = 1 pre; 0 post
        uint32_t I : 1;         // [25] = 1 for immediate instruction
        uint32_t zeroone : 2;   // [27:26] = 01b
        uint32_t cond : 4;      // [31:28]
    } reg_bits;
    struct imm_bits_type {
        uint32_t imm : 12;      // [11:0]
        uint32_t rd : 4;        // [15:12]
        uint32_t rn : 4;        // [19:16]
        uint32_t L : 1;         // [20] = 1 load; 0 store
        uint32_t W : 1;         // [21] = 1 wr addr into base; 0 no write-back
        uint32_t B : 1;         // [22] = 1 byte; 0 word
        uint32_t U : 1;         // [23] = 1 add offset; 0 subtruct offset
        uint32_t P : 1;         // [24] = 1 pre / 0 post
        uint32_t I : 1;         // [25] = 1 for immediate instruction
        uint32_t zeroone : 2;   // [27:26] = 01b
        uint32_t cond : 4;      // [31:28]
    } imm_bits;
    uint32_t value;
};

union HWordSignedDataTransferType {
    struct reg_bits_type {
        uint32_t rm : 4;        // [3:0] offset register
        uint32_t b4 : 1;        // [4] =1
        uint32_t h : 1;         // [5] 0=byte; 1=half-word
        uint32_t s : 1;         // [6] 0=/unsigned; 1=signed
        uint32_t b7 : 1;        // [7] =1
        uint32_t imm_h : 4;     // [11:8] zero/immediate high nibble
        uint32_t rd : 4;        // [15:12]
        uint32_t rn : 4;        // [19:16]
        uint32_t L : 1;         // [20] = 1 load; 0 store
        uint32_t W : 1;         // [21] = 1 wr addr into base; 0 no write-back
        uint32_t reg_imm : 1;   // [22] = 0=reg offset; 1=imm offset
        uint32_t U : 1;         // [23] = 1 add offset; 0 subtruct offset
        uint32_t P : 1;         // [24] = 1 pre; 0 post
        uint32_t zero3 : 3;     // [27:25] = 000b
        uint32_t cond : 4;      // [31:28]
    } bits;
    uint32_t value;
};

union DWordDataTransferType {
    struct reg_bits_type {
        uint32_t rm : 4;        // [3:0] imm4L/offset register
        uint32_t b1 : 1;        // [4] =1
        uint32_t ld_st : 1;     // [5] =0=load; 1=store
        uint32_t b11 : 2;       // [7:6] =11
        uint32_t imm_h : 4;     // [11:8] zero/immediate high nibble
        uint32_t rt : 4;        // [15:12]
        uint32_t rn : 4;        // [19:16]
        uint32_t L : 1;         // [20] = 1 load; 0 store
        uint32_t W : 1;         // [21] = 1 wr addr into base; 0 no write-back
        uint32_t reg_imm : 1;   // [22] = 0=reg offset; 1=imm offset
        uint32_t U : 1;         // [23] = 1 add offset; 0 subtruct offset
        uint32_t P : 1;         // [24] = 1 pre; 0 post
        uint32_t zero3 : 3;     // [27:25] = 000b
        uint32_t cond : 4;      // [31:28]
    } bits;
    uint32_t value;
};

union BitsFieldType {
    struct reg_bits_type {
        uint32_t rn : 4;        // [3:0] source register
        uint32_t b6_4 : 3;      // [6:4] =001
        uint32_t lsb : 5;       // [11:7] destination bit 0 to 31
        uint32_t rd : 4;        // [15:12] destination register
        uint32_t msb : 5;       // [20:16]
        uint32_t b27_21 : 7;    // [27:21] = 0111110b
        uint32_t cond : 4;      // [31:28]
    } bits;
    uint32_t value;
};

union CoprocessorTransferType {
    struct bits_type {
        uint32_t crm : 4;       // [3:0] Coproc. operand register
        uint32_t one : 1;       // 1b
        uint32_t cp_nfo : 3;    // [11:5] Coproc. information
        uint32_t cp_num : 4;    // [11:8] Coproc. number
        uint32_t rd : 4;        // [15:12] Dest. register
        uint32_t crn : 4;       // [19:16] Coproc.src/dest reg.
        uint32_t L : 1;         // [20] 1 load; 0 store
        uint32_t mode : 3;      // [23:21] Coproc. operation mode
        uint32_t opcode : 4;    // [27:24] = 1110b
        uint32_t cond : 4;      // [31:28]
    } bits;
    uint32_t value;
};

union PsrTransferType {
    struct reg_bits_type {
        uint32_t rm : 4;        // [3:0] source reg
        uint32_t zero : 8;      // [15:4] =00000000b
        uint32_t rd : 4;        // [15:12]
        uint32_t bitmask : 4;   // [19:16] 
        uint32_t b21_20 : 18;   // [21:20] =10b
        uint32_t Pd : 1;        // [23] = 0=CPSR; 1=SPSR_mode
        uint32_t b24_23 : 2;    // [24:23] = 10b
        uint32_t I : 1;         // [25] = 1 for immediate instruction
        uint32_t b27_26 : 2;    // [27:26] = 00b
        uint32_t cond : 4;      // [31:28]
    } reg_bits;
    struct imm_bits_type {
        uint32_t imm : 8;       // [7:0]
        uint32_t rotate : 4;    // [11:8] shift applied to imm
        uint32_t rd : 4;        // [15:12]
        uint32_t bitmask : 4;   // [19:16] 
        uint32_t b21_20 : 18;   // [21:20] =10b
        uint32_t Pd : 1;        // [23] = 0=CPSR; 1=SPSR_mode
        uint32_t b24_23 : 2;    // [24:23] = 10b
        uint32_t I : 1;         // [25] = 1 for immediate instruction
        uint32_t b27_26 : 2;    // [27:26] = 00b
        uint32_t cond : 4;      // [31:28]
    } imm_bits;
    uint32_t value;
};

union BranchType {
    struct bits_type {
        uint32_t offset : 24;   // [23:0] offset
        uint32_t L : 1;         // [24] 0 branch; 1 branch with link
        uint32_t opcode : 3;    // [27:25] = 101b
        uint32_t cond : 4;      // [31:28]
    } bits;
    uint32_t value;
};

union BranchExchangeIndirectType {
    struct bits_type {
        uint32_t rm : 4;        // [3:0]
        uint32_t opcode0 : 4;   // [4:7] = 0011b
        uint32_t SBO : 12;      // [8:19] = 111111111111b
        uint32_t opcode2 : 8;   // [27:25] = 00010010b
        uint32_t cond : 4;      // [31:28]
    } bits;
    uint32_t value;
};

union BlockDataTransferType {
    struct bits_type {
        uint32_t reglist : 16;  // [15:0] Register list
        uint32_t rn : 4;        // [19:16] base register
        uint32_t L : 1;         // [20] 0=load; 1=store
        uint32_t W : 1;         // [21] 0=no write-back; 1=write adr into base
        uint32_t S : 1;         // [22] PSR & force user bit
        uint32_t U : 1;         // [23] 0=down; 1=up adr. increment
        uint32_t P : 1;         // [24] 0=post; 1=pre-increment
        uint32_t b27_25 : 3;    // [27:25] = 100b
        uint32_t cond : 4;      // [31:28]
    } bits;
    uint32_t value;
};

union SignExtendType {
    struct bits_type {
        uint32_t rm : 4;        // [3:0]
        uint32_t b7_4 : 4;      // [7:4] 0111b
        uint32_t sbz : 2;       // [9:8]
        uint32_t rotate : 2;    // [11:10] 0=0; 1=ror8; 2=ror16; 3=ror24
        uint32_t rd : 4;        // [15:12]
        uint32_t rn : 4;        // [19:16]
        uint32_t b27_20 : 8;    // [27:16] = 01101110b
        uint32_t cond : 4;      // [31:28]
    } bits;
    uint32_t value;
};

union DivType {
    struct div_bits_type {
        uint32_t rn : 4;        //[3:0]
        uint32_t b7_4 : 4;      //[7:4] = 0001b
        uint32_t rm : 4;        //[11:8]
        uint32_t b15_12 : 4;    //[15:12] = 1111b
        uint32_t rd : 4;        //[19:16]
        uint32_t b20 : 1;       //[20]
        uint32_t S : 1;         //[21] = 1b
        uint32_t b27_22 : 6;    //[27:22] = 011100b
        uint32_t cond : 4;      //[31:28]
    } bits;
    uint32_t value;
};

union MulType {
    struct mul_bits_type {
        uint32_t rm : 4;        //[3:0]
        uint32_t b7_4 : 4;      //[7:4] = 1001b
        uint32_t rs : 4;        //[11:8]
        uint32_t rn : 4;        //[15:12]
        uint32_t rd : 4;        //[19:16]
        uint32_t S : 1;         //[20]
        uint32_t A : 1;         //[21]
        uint32_t b27_22 : 6;    //[27:22]
        uint32_t cond : 4;      //[31:28]
    } bits;
    uint32_t value;
};

union MulLongType {
    struct mull_bits_type {
        uint32_t rm : 4;        //[3:0]
        uint32_t b7_4 : 4;      //[7:4] = 1001b
        uint32_t rs : 4;        //[11:8]
        uint32_t rdlo : 4;      //[15:12]
        uint32_t rdhi : 4;      //[19:16]
        uint32_t S : 1;         //[20] 0=do not alter condition codes
        uint32_t A : 1;         //[21] 0=mul only; 1=mul + accumulate
        uint32_t U : 1;         //[22] 0=unsigned; 1=signed
        uint32_t b27_22 : 5;    //[27:21] = 00001b
        uint32_t cond : 4;      //[31:28]
    } bits;
    uint32_t value;
};


union ProgramStatusRegsiterType {
    struct bits_type {
        uint32_t M : 5;         // [4:0] CPU mode: 0x13=supervisor
        uint32_t T : 1;         // [5] 0=ARM mode; 1=Thumb mode
        uint32_t F : 1;         // [6] 1=FIQ disable; 0=FIQ enable
        uint32_t I : 1;         // [7] 1=IRQ disable; 0=IRQ enable
        uint32_t A : 1;         // [8] 1=disable imprecise data aborts
        uint32_t E : 1;         // [9] Endianess
        uint32_t b15_10 : 6;    // [15:10] reserved
        uint32_t GE : 4;        // [19:16] Greater than or Equal
        uint32_t b23_20 : 4;    // [23:20] reserved
        uint32_t J : 1;         // [24] 1=Jazelle ISA; 0=reserved
        uint32_t b26_25 : 2;    // [26:25] reserved
        uint32_t Q : 1;         // [27] overflow in DSP instruction
        uint32_t V : 1;         // [28] overflow flag
        uint32_t C : 1;         // [29] carry flag
        uint32_t Z : 1;         // [30] zero flag
        uint32_t N : 1;         // [31] negative flag
    } u;
    uint32_t value;
};

static const char *const ARM_IREGS_NAMES[] = {
    "r0",       // [0]
    "r1",       // [1] 
    "r2",       // [2] 
    "r3",       // [3] 
    "r4",       // [4] 
    "r5",       // [5] 
    "r6",       // [6] 
    "r7",       // [7] 
    "r8",       // [8] 
    "r9",       // [9] 
    "sl",       // [10]
    "r11",      // [11]
    "fp",       // [12] frame pointer
    "sp",       // [13] stack pointer
    "lr",       // [14] link register
    "pc",       // [15] instruction pointer
    "cpsr",     // [16] Current Prog. Status Reg (all modes)
    "spsr",     // [17] Saved Prog. Status Reg
};

static const ECpuRegMapping ARM_DEBUG_REG_MAP[] = {
    {"npc",   4, 0x7b1},
    {"steps", 8, 0xC02},
    {"r0",    4, 0x1000},
    {"r1",    4, 0x1001},
    {"r2",    4, 0x1002},
    {"r3",    4, 0x1003},
    {"r4",    4, 0x1004},
    {"r5",    4, 0x1005},
    {"r6",    4, 0x1006},
    {"r7",    4, 0x1007},
    {"r8",    4, 0x1008},
    {"r9",    4, 0x1009},
    {"r10",   4, 0x100A},
    {"r11",   4, 0x100B},
    {"fp",    4, 0x100C},
    {"sp",    4, 0x100D},
    {"lr",    4, 0x100E},
    {"cpsr",  4, 0x100F},
    {"",      0, 0}
};


enum EConditionSuffix {
    Cond_EQ,    // equal
    Cond_NE,    // not equal
    Cond_CS,    // unsigned higer or same
    Cond_CC,    // unsigned lower
    Cond_MI,    // negative
    Cond_PL,    // positive or zero
    Cond_VS,    // Overflow
    Cond_VC,    // no overflow
    Cond_HI,    // unsigned higher
    Cond_LS,    // unsigned lower or same
    Cond_GE,    // greater or equal
    Cond_LT,    // less than
    Cond_GT,    // greater than
    Cond_LE,    // less tha or equal
    Cond_AL,    // always
};

enum EArmRegNames {
    Reg_r0,
    Reg_r1,       // [1] Return address
    Reg_r2,       // [2] Stack pointer
    Reg_r3,       // [3] Global pointer
    Reg_r4,       // [4] Thread pointer
    Reg_r5,       // [5] Temporaries 0 s3
    Reg_r6,       // [6] Temporaries 1 s4
    Reg_r7,       // [7] Temporaries 2 s5
    Reg_r8,       // [8] s0/fp Saved register/frame pointer
    Reg_r9,       // [9] Saved register 1
    Reg_r10,      // [10] Function argumentes 0
    Reg_r11,      // [11] Function argumentes 1
    Reg_fe,       // [12] Function argumentes 2
    Reg_sp,       // [13] Function argumentes 3
    Reg_lr,       // [14] Function argumentes 4
    Reg_pc,       // [15] instruction pointer
    Reg_cpsr,     // [16] Current Prog. Status Reg (all modes)
    Reg_spsr,     // [17] Saved Prog. Status Reg
    Reg_rsrv18,
    Reg_rsrv19,
    Reg_rsrv20,
    Reg_rsrv21,
    Reg_rsrv22,
    Reg_rsrv23,
    Reg_rsrv24,
    Reg_rsrv25,
    Reg_rsrv26,
    Reg_rsrv27,
    Reg_rsrv28,
    Reg_rsrv29,
    Reg_rsrv30,
    Reg_rsrv31,
    Reg_Total
};

enum EIsaArmV7 {
    ARMV7_B,
    ARMV7_BL,
    ARMV7_BX,
    ARMV7_BLX,
    ARMV7_AND,
    ARMV7_EOR,
    ARMV7_SUB,
    ARMV7_RSB,
    ARMV7_ADD,
    ARMV7_ADC,
    ARMV7_SBC,
    ARMV7_RSC,
    ARMV7_TST,
    ARMV7_TEQ,
    ARMV7_CMP,
    ARMV7_CMN,
    ARMV7_ORR,
    ARMV7_MOV,
    ARMV7_BIC,
    ARMV7_MVN,
    ARMV7_MRS,
    ARMV7_MSR,
    ARMV7_MUL,
    ARMV7_MLA,
    ARMV7_UMULL,
    ARMV7_UMLAL,
    ARMV7_SMULL,
    ARMV7_SMLAL,
    ARMV7_LDR,
    ARMV7_LDRB,
    ARMV7_STR,
    ARMV7_STRB,
    ARMV7_SWP,
    ARMV7_LDRH,
    ARMV7_LDRSB,
    ARMV7_LDRSH,
    ARMV7_STRH,
    ARMV7_LDM,
    ARMV7_STM,
    ARMV7_SWI,
    ARMV7_MRC,
    ARMV7_MCR,
    ARMV7_LDRD,
    ARMV7_STRD,
    ARMV7_UXTB,
    ARMV7_UXTAB,
    ARMV7_UXTB16,
    ARMV7_UXTAB16,
    ARMV7_UXTH,
    ARMV7_UXTAH,
    ARMV7_NOP,
    ARMV7_MOVT,
    ARMV7_MOVW,
    ARMV7_UDIV,
    ARMV7_SDIV,
    ARMV7_BFC,
    ARMV7_BFI,
    T1_ADC_I,   // 4.6.1
    T1_ADD_I,   // 4.6.3
    T2_ADD_I,   // 4.6.3
    T3_ADD_I,   // 4.6.3
    T1_ADD_R,   // 4.6.4
    T2_ADD_R,   // 4.6.4
    T3_ADD_R,   // 4.6.4
    T1_ADDSP_I, // 4.6.5
    T2_ADDSP_I, // 4.6.5
    T1_ADR,     // 4.6.7
    T1_AND_I,   // 4.6.8
    T1_AND_R,   // 4.6.9
    T2_AND_R,   // 4.6.9
    T1_ASR_I,   // 4.6.10
    T1_B,       // 4.6.12
    T2_B,       // 4.6.12
    T3_B,       // 4.6.12
    T4_B,       // 4.6.12
    T1_BIC_I,   // 4.6.15
    T1_BKPT,    // 4.6.17
    T1_BL_I,    // 4.6.18
    T1_BLX_R,   // 4.6.19
    T1_BX,      // 4.6.20
    T1_CBNZ,    // 4.6.22
    T1_CBZ,     // 4.6.23
    T1_CMP_I,   // 4.6.29
    T2_CMP_I,   // 4.6.29
    T1_CMP_R,   // 4.6.30
    T2_CMP_R,   // 4.6.30
    T1_CPS,     // 4.6.31
    T1_EOR_I,   // 4.6.36
    T1_EOR_R,   // 4.6.37
    T1_IT,      // 4.6.39
    T2_LDMIA,   // 4.6.42
    T1_LDR_I,   // 4.6.43
    T2_LDR_I,   // 4.6.43
    T3_LDR_I,   // 4.6.43
    T4_LDR_I,   // 4.6.43
    T1_LDR_L,   // 4.6.44
    T2_LDR_L,   // 4.6.44
    T1_LDR_R,   // 4.6.45
    T2_LDR_R,   // 4.6.45
    T1_LDRB_I,  // 4.6.46
    T2_LDRB_I,  // 4.6.46
    T3_LDRB_I,  // 4.6.46
    T1_LDRB_R,  // 4.6.48
    T2_LDRB_R,  // 4.6.48
    T1_LDRH_I,  // 4.6.55
    T2_LDRH_R,  // 4.6.57
    T1_LDRSB_I, // 4.6.59
    T1_LDRSB_R, // 4.6.61
    T2_LDRSB_R, // 4.6.61
    T1_LSL_I,   // 4.6.68
    T1_LSL_R,   // 4.6.69
    T2_LSL_R,   // 4.6.69
    T1_LSR_I,   // 4.6.70
    T1_LSR_R,   // 4.6.71
    T2_LSR_R,   // 4.6.71
    T1_MLA,     // 4.6.74
    T1_MLS,     // 4.6.75
    T1_MOV_I,   // 4.6.76
    T2_MOV_I,   // 4.6.76
    T3_MOV_I,   // 4.6.76
    T1_MOV_R,   // 4.6.77
    T2_MOV_R,   // 4.6.77
                // 4.6.78 MOV (shifted register) is a synonym for ASR, LSL, LSR, ROR, RRX
    T1_MUL,     // 4.6.84
    T2_MUL,     // 4.6.84
    T1_MVN_R,   // 4.6.86
    T1_NOP,     // 4.6.88
                // 4.6.87 NEG is a pre-UAL synonym for RSB (immediate) with imm=0
    T1_ORR_I,   // 4.6.91
    T1_ORR_R,   // 4.6.92
    T2_ORR_R,   // 4.6.92
    T1_POP,     // 4.6.98
    T2_POP,     // 4.6.98
    T1_PUSH,    // 4.6.99, T2_PUSH equivalen STMDB SP!,<register>
    T1_RSB_I,   // 4.6.119
    T2_RSB_I,   // 4.6.119
    T1_RSB_R,   // 4.6.119
    T1_SBFX,    // 4.6.125
    T1_SDIV,    // 4.6.126
    T1_SMULL,   // 4.6.150
    T1_STMDB,   // 4.6.160
    T1_STMIA,   // 4.6.161
    T1_STR_I,   // 4.6.162
    T2_STR_I,   // 4.6.162
    T1_STR_R,   // 4.6.163
    T2_STR_R,   // 4.6.163
    T1_STRB_I,  // 4.6.164
    T2_STRB_I,  // 4.6.164
    T3_STRB_I,  // 4.6.164
    T1_STRB_R,  // 4.6.165
    T2_STRB_R,  // 4.6.165
    T1_STRD_I,  // 4.6.167
    T1_STRH_I,  // 4.6.172
    T2_STRH_I,  // 4.6.172
    T3_STRH_I,  // 4.6.172
    T1_SUB_I,   // 4.6.176
    T2_SUB_I,   // 4.6.176
    T3_SUB_I,   // 4.6.176
    T1_SUB_R,   // 4.6.177
    T2_SUB_R,   // 4.6.177
    T1_SUBSP_I, // 4.6.178
    T1_SXTAB,   // 4.6.182
    T1_SXTB,    // 4.6.185
    T1_TBB,     // 4.6.188
    T1_TST_I,   // 4.6.192
    T1_TST_R,   // 4.6.193
    T1_UBFX,    // 4.6.197
    T1_UDIV,    // 4.6.198
    T1_UMULL,   // 4.6.207
    T1_UXTAB,   // 4.6.221
    T1_UXTAH,   // 4.6.223
    T1_UXTB,    // 4.6.224
    T1_UXTH,    // 4.6.226
    ARMV7_Total
};

enum EArmInstructionModes {
    ARM_mode,
    THUMB_mode,
    Jazelle_mode,
    ArmInstrModes_Total = 4
};

enum EArmCoreModes {
    ArmUser_mode = 0x10,
    ArmFIQ_mode = 0x11,
    ArmIRQ_mode = 0x12,
    ArmSupervisor_mode = 0x13,
    ArmAbort_mode = 0x17,
    ArmUndefined_mode = 0x1B,
    ArmSystem_mode = 0x1F,
    ArmCoreModes_Total = 32
};

enum EArmM4Modes {
    ArmM4_MainMode,
    ArmM4_HandlerMode,
    ArmM4_ThreadMode,
};

enum SRType {
    SRType_None,
    SRType_LSL,
    SRType_LSR,
    SRType_ASR,
    SRType_ROR,
    SRType_RRX
};

/**
    0xFFFFFFF1 Return to Handler mode. Use MSP and return to MSP
    0xFFFFFFF9 Return to Thread mode. Use MSP and return to MSP
    0xFFFFFFFD Return to Thread mode. Use PSP and return to PSP
 */
union ARM_EXC_RETURN {
    uint32_t v;
    struct bits_type {
        uint32_t code : 5;      // [4:0]
        uint32_t ones : 27;     // [31:5] = `1
    } b;
};

/** Internal simulation bits only */
static const uint64_t Interrupt_SoftwareIdx = 0;

}  // namespace debugger

#endif  // __DEBUGGER_COMMON_ARM_ISA_H__
