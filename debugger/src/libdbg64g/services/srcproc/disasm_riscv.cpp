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

#include <riscv-isa.h>
#include <attribute.h>
#include <api_core.h>
#include "coreservices/icpuriscv.h"
#include <inttypes.h>
#include <iostream>

namespace debugger {

static const char *const *RN = RISCV_IREGS_NAMES;
static const char *const *RF = &RISCV_IREGS_NAMES[ICpuRiscV::RegFpu_Offset];

typedef int (*disasm_opcode_f)(uint64_t pc, uint32_t code,
                              AttributeType *mnemonic, AttributeType *comment);

typedef int (*disasm_opcode16_f)(uint64_t pc,
                                Reg16Type code, AttributeType *mnemonic,
                                AttributeType *comment);

int opcode_0x00(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x01(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x03(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x04(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x05(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x06(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x08(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x09(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x0B(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x0C(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x0D(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x0E(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x14(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x18(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x19(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x1B(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);
int opcode_0x1C(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment);

int C_ADDI16SP_LUI(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_ADDI4SPN(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_BEQZ(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_BNEZ(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_J(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_JAL_ADDIW(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_JR_MV_EBREAK_JALR_ADD(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_LD(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_LI(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_LDSP(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_LW(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_LWSP(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_MATH(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_NOP_ADDI(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_SD(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_SDSP(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_SLLI(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_SW(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);
int C_SWSP(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment);


static const disasm_opcode_f tblOpcode1_[32] = {
    &opcode_0x00, // 0
    &opcode_0x01, // 1
    0, // 2
    &opcode_0x03, // 3
    &opcode_0x04, // 4
    &opcode_0x05, // 5
    &opcode_0x06, // 6
    0, // 7
    &opcode_0x08, // 8
    &opcode_0x09, // 9
    0, // 0x0A
    &opcode_0x0B, // 0x0B
    &opcode_0x0C, // 0x0C
    &opcode_0x0D, // 0x0D
    &opcode_0x0E, // 0x0E
    0, // 0x0F
    0, // 0x10
    0, // 0x11
    0, // 0x12
    0, // 0x13
    &opcode_0x14, // 0x14
    0, // 0x15
    0, // 0x16
    0, // 0x17
    &opcode_0x18, // 0x18
    &opcode_0x19, // 0x19
    0, // 0x1A
    &opcode_0x1B, // 0x1B
    &opcode_0x1C, // 0x1C
    0, // 0x1D
    0, // 0x1E
    0 // 0x1F
};

static const disasm_opcode16_f tblCompressed_[32] = {
    &C_ADDI4SPN, // 0x00
    &C_NOP_ADDI, // 0x01
    &C_SLLI, // 0x02
    0, // 0x03
    0, // 0x04
    &C_JAL_ADDIW, // 0x05
    0, // 0x06
    0, // 0x07
    &C_LW, // 0x08
    &C_LI, // 0x09
    &C_LWSP, // 0x0A
    0, // 0x0B
    &C_LD, // 0x0C
    &C_ADDI16SP_LUI, // 0x0D
    &C_LDSP, // 0x0E
    0, // 0x0F
    0, // 0x10
    &C_MATH, // 0x11
    &C_JR_MV_EBREAK_JALR_ADD, // 0x12
    0, // 0x13
    0, // 0x14
    &C_J, // 0x15
    0, // 0x16
    0, // 0x17
    &C_SW, // 0x18
    &C_BEQZ, // 0x19
    &C_SWSP, // 0x1A
    0, // 0x1B
    &C_SD, // 0x1C
    &C_BNEZ, // 0x1D
    &C_SDSP, // 0x1E
    0 // 0x1F
};

int disasm_riscv(uint64_t pc,
                uint8_t *data,
                int offset,
                AttributeType *mnemonic,
                AttributeType *comment) {
    int oplen;
    if ((data[offset] & 0x3) < 3) {
        Reg16Type val;
        uint32_t hash;
        val.word = *reinterpret_cast<uint16_t*>(&data[offset]);
        hash = ((val.word >> 11) & 0x1C) | (val.word & 0x3);
        oplen = 2;
        if (tblCompressed_[hash]) {
            return tblCompressed_[hash](pc + static_cast<uint64_t>(offset),
                                        val,
                                        mnemonic,
                                        comment);
        }
    } else if ((data[offset] & 0x3) == 0x3) {
        uint32_t val = *reinterpret_cast<uint32_t*>(&data[offset]);
        uint32_t opcode1 = (val >> 2) & 0x1f;
        oplen = 4;
        if (tblOpcode1_[opcode1]) {
            return tblOpcode1_[opcode1](pc + static_cast<uint64_t>(offset),
                                        val,
                                        mnemonic,
                                        comment);
        }
    }

    mnemonic->make_string("unimpl");
    comment->make_string("");
    return oplen;
}


int opcode_0x00(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_I_type i;
    int32_t imm;

    i.value = code;
    imm = static_cast<int32_t>(code) >> 20;
    switch (i.bits.funct3) {
    case 0:
        RISCV_sprintf(tstr, sizeof(tstr), "lb      %s,%d(%s)",
            RN[i.bits.rd], imm, RN[i.bits.rs1]);
        break;
    case 1:
        RISCV_sprintf(tstr, sizeof(tstr), "lh      %s,%d(%s)",
            RN[i.bits.rd], imm, RN[i.bits.rs1]);
        break;
    case 2:
        RISCV_sprintf(tstr, sizeof(tstr), "lw      %s,%d(%s)",
            RN[i.bits.rd], imm, RN[i.bits.rs1]);
        break;
    case 3:
        RISCV_sprintf(tstr, sizeof(tstr), "ld      %s,%d(%s)",
            RN[i.bits.rd], imm, RN[i.bits.rs1]);
        break;
    case 4:
        RISCV_sprintf(tstr, sizeof(tstr), "lbu     %s,%d(%s)",
            RN[i.bits.rd], imm, RN[i.bits.rs1]);
        break;
    case 5:
        RISCV_sprintf(tstr, sizeof(tstr), "lhu     %s,%d(%s)",
            RN[i.bits.rd], imm, RN[i.bits.rs1]);
        break;
    case 6:
        RISCV_sprintf(tstr, sizeof(tstr), "lwu     %s,%d(%s)",
            RN[i.bits.rd], imm, RN[i.bits.rs1]);
        break;
    default:;
    }
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x01(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_I_type i;
    int32_t imm;

    i.value = code;
    imm = static_cast<int32_t>(code) >> 20;
    switch (i.bits.funct3) {
    case 3:
        RISCV_sprintf(tstr, sizeof(tstr), "fld     %s,%d(%s)",
            RF[i.bits.rd], imm, RN[i.bits.rs1]);
        break;
    default:;
    }
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x03(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_R_type r;

    r.value = code;
    switch (r.bits.funct3) {
    case 0:
        RISCV_sprintf(tstr, sizeof(tstr), "%s", "fence");
        break;
    case 1:
        RISCV_sprintf(tstr, sizeof(tstr), "%s", "fence_i");
        break;
    default:;
    }
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x04(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_I_type i;
    int32_t imm;

    i.value = code;
    imm = static_cast<int32_t>(code) >> 20;
    switch (i.bits.funct3) {
    case 0:
        if (imm == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "mv      %s,%s",
                RN[i.bits.rd], RN[i.bits.rs1]);
        } else if (i.bits.rs1 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "li      %s,%d",
                RN[i.bits.rd], imm);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "addi    %s,%s,%d",
                RN[i.bits.rd], RN[i.bits.rs1], imm);
        }
        break;
    case 1:
        RISCV_sprintf(tstr, sizeof(tstr), "slli    %s,%s,%d",
            RN[i.bits.rd], RN[i.bits.rs1], imm);
        break;
    case 2:
        RISCV_sprintf(tstr, sizeof(tstr), "slti    %s,%s,%d",
            RN[i.bits.rd], RN[i.bits.rs1], imm);
        break;
    case 3:
        RISCV_sprintf(tstr, sizeof(tstr), "sltiu   %s,%s,%d",
            RN[i.bits.rd], RN[i.bits.rs1], imm);
        break;
    case 4:
        RISCV_sprintf(tstr, sizeof(tstr), "xori    %s,%s,%d",
            RN[i.bits.rd], RN[i.bits.rs1], imm);
        break;
    case 5:
        if ((code >> 26) == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "srli    %s,%s,%d",
                RN[i.bits.rd], RN[i.bits.rs1], imm);
        } else if ((code >> 26) == 0x20) {
            RISCV_sprintf(tstr, sizeof(tstr), "srai    %s,%s,%d",
                RN[i.bits.rd], RN[i.bits.rs1], imm);
        }
        break;
    case 6:
        RISCV_sprintf(tstr, sizeof(tstr), "ori     %s,%s,%d",
            RN[i.bits.rd], RN[i.bits.rs1], imm);
        break;
    case 7:
        RISCV_sprintf(tstr, sizeof(tstr), "andi    %s,%s,%d",
            RN[i.bits.rd], RN[i.bits.rs1], imm);
        break;
    default:;
    }
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x05(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_U_type u;
    uint64_t imm64;

    u.value = code;
    imm64 = u.bits.imm31_12 << 12;
    if (imm64 & (1LL << 31)) {
        imm64 |= EXT_SIGN_32;
    }
    RISCV_sprintf(tstr, sizeof(tstr), "auipc   %s,0x%" RV_PRI64 "x",
        RN[u.bits.rd], imm64);
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x06(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_I_type i;
    int32_t imm;

    i.value = code;
    imm = static_cast<int32_t>(code) >> 20;
    switch (i.bits.funct3) {
    case 0:
        RISCV_sprintf(tstr, sizeof(tstr), "addiw   %s,%s,%d",
            RN[i.bits.rd], RN[i.bits.rs1], imm);
        break;
    case 1:
        RISCV_sprintf(tstr, sizeof(tstr), "slliw   %s,%s,%d",
            RN[i.bits.rd], RN[i.bits.rs1], imm);
        break;
    case 5:
        if ((code >> 25) == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "srliw   %s,%s,%d",
                RN[i.bits.rd], RN[i.bits.rs1], imm);
        } else if ((code >> 25) == 0x20) {
            RISCV_sprintf(tstr, sizeof(tstr), "sraiw   %s,%s,%d",
                RN[i.bits.rd], RN[i.bits.rs1], imm);
        }
        break;
    default:;
    }
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x08(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_S_type s;
    int32_t imm;
    s.value = code;
    imm = (s.bits.imm11_5 << 5) | s.bits.imm4_0;
    if (imm & 0x800) {
        imm |= EXT_SIGN_12;
    }
    switch (s.bits.funct3) {
    case 0:
        RISCV_sprintf(tstr, sizeof(tstr), "sb      %s,%d(%s)",
            RN[s.bits.rs2], imm, RN[s.bits.rs1]);
        break;
    case 1:
        RISCV_sprintf(tstr, sizeof(tstr), "sh      %s,%d(%s)",
            RN[s.bits.rs2], imm, RN[s.bits.rs1]);
        break;
    case 2:
        RISCV_sprintf(tstr, sizeof(tstr), "sw      %s,%d(%s)",
            RN[s.bits.rs2], imm, RN[s.bits.rs1]);
        break;
    case 3:
        RISCV_sprintf(tstr, sizeof(tstr), "sd      %s,%d(%s)",
            RN[s.bits.rs2], imm, RN[s.bits.rs1]);
        break;
    default:;
    }
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x09(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_S_type s;
    int32_t imm;
    s.value = code;
    imm = (s.bits.imm11_5 << 5) | s.bits.imm4_0;
    if (imm & 0x800) {
        imm |= EXT_SIGN_12;
    }
    switch (s.bits.funct3) {
    case 3:
        RISCV_sprintf(tstr, sizeof(tstr), "fsd     %s,%d(%s)",
            RF[s.bits.rs2], imm, RN[s.bits.rs1]);
        break;
    default:;
    }
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

// Atomic Memory Operations (AMO)
int opcode_0x0B(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    const char *aquired[2] = {"", ".aq"};
    const char *released[2] = {"", ".rl"};
    size_t tlen = 0;
    ISA_R_type r;
    r.value = code;
    switch (r.amobits.funct3) {
    case 2: // 32-bits
        switch (r.amobits.funct5) {
        case 0x00:
            tlen = RISCV_sprintf(tstr, sizeof(tstr), "%s", "amoadd_w");
            break;
        case 0x01:
            tlen = RISCV_sprintf(tstr, sizeof(tstr), "%s", "amoswap_w");
            break;
        case 0x02:
            if (r.amobits.rs2 == 0) {
                tlen = RISCV_sprintf(tstr, sizeof(tstr), "%s", "lr_w");
            }
            break;
        case 0x03:
            tlen = RISCV_sprintf(tstr, sizeof(tstr), "%s", "sc_w");
            break;
        case 0x04:
            tlen = RISCV_sprintf(tstr, sizeof(tstr), "%s", "amoxor_w");
            break;
        case 0x08:
            tlen = RISCV_sprintf(tstr, sizeof(tstr), "%s", "amoor_w");
            break;
        case 0x0C:
            tlen = RISCV_sprintf(tstr, sizeof(tstr), "%s", "amoand_w");
            break;
        case 0x10:
            tlen = RISCV_sprintf(tstr, sizeof(tstr), "%s", "amomin_w");
            break;
        case 0x14:
            tlen = RISCV_sprintf(tstr, sizeof(tstr), "%s", "amomax_w");
            break;
        case 0x18:
            tlen = RISCV_sprintf(tstr, sizeof(tstr), "%s", "amominu_w");
            break;
        case 0x1C:
            tlen = RISCV_sprintf(tstr, sizeof(tstr), "%s", "amomaxu_w");
            break;
        default:;
        }
        break;
    case 3: // RV64 only
        switch (r.amobits.funct5) {
        case 0x00:
            tlen = RISCV_sprintf(tstr, sizeof(tstr), "%s", "amoadd_d");
            break;
        case 0x01:
            tlen = RISCV_sprintf(tstr, sizeof(tstr), "%s", "amoswap_d");
            break;
        case 0x02:
            if (r.amobits.rs2 == 0) {
                tlen = RISCV_sprintf(tstr, sizeof(tstr), "%s", "lr_d");
            }
            break;
        case 0x03:
            tlen = RISCV_sprintf(tstr, sizeof(tstr), "%s", "sc_d");
            break;
        case 0x04:
            tlen = RISCV_sprintf(tstr, sizeof(tstr), "%s", "amoxor_d");
            break;
        case 0x08:
            tlen = RISCV_sprintf(tstr, sizeof(tstr), "%s", "amoor_d");
            break;
        case 0x0C:
            tlen = RISCV_sprintf(tstr, sizeof(tstr), "%s", "amoand_d");
            break;
        case 0x10:
            tlen = RISCV_sprintf(tstr, sizeof(tstr), "%s", "amomin_d");
            break;
        case 0x14:
            tlen = RISCV_sprintf(tstr, sizeof(tstr), "%s", "amomax_d");
            break;
        case 0x18:
            tlen = RISCV_sprintf(tstr, sizeof(tstr), "%s", "amominu_d");
            break;
        case 0x1C:
            tlen = RISCV_sprintf(tstr, sizeof(tstr), "%s", "amomaxu_d");
            break;
        default:;
        }
        break;
    default:;
    }
    if (tlen) {
        RISCV_sprintf(&tstr[tlen], sizeof(tstr)-tlen, "%s%s %s,%s,(%s)",
            aquired[r.amobits.aq],
            released[r.amobits.rl],
            RN[r.amobits.rd], RN[r.amobits.rs2], RN[r.amobits.rs1]);
    }
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x0C(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_R_type r;
    r.value = code;
    switch (r.bits.funct3) {
    case 0:
        if (r.bits.funct7 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "add     %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        } else if (r.bits.funct7 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "mul     %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        } else if (r.bits.funct7 == 0x20) {
            RISCV_sprintf(tstr, sizeof(tstr), "sub     %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        }
        break;
    case 1:
        RISCV_sprintf(tstr, sizeof(tstr), "sll     %s,%s,%s",
            RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        break;
    case 2:
        RISCV_sprintf(tstr, sizeof(tstr), "slt     %s,%s,%s",
            RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        break;
    case 3:
        RISCV_sprintf(tstr, sizeof(tstr), "sltu     %s,%s,%s",
            RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        break;
    case 4:
        if (r.bits.funct7 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "xor     %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        } else if (r.bits.funct7 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "div     %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        }
        break;
    case 5:
        if (r.bits.funct7 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "srl     %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        } else if (r.bits.funct7 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "divu    %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        } else if (r.bits.funct7 == 0x20) {
            RISCV_sprintf(tstr, sizeof(tstr), "sra     %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        }
        break;
    case 6:
        if (r.bits.funct7 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "or      %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        } else if (r.bits.funct7 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "rem     %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        }
        break;
    case 7:
        if (r.bits.funct7 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "and     %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        } else if (r.bits.funct7 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "remu    %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        }
        break;
    default:;
    }
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x0D(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_U_type u;
    u.value = code;
    RISCV_sprintf(tstr, sizeof(tstr), "lui     %s,0x%x",
        RN[u.bits.rd], u.bits.imm31_12);
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x0E(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_R_type r;

    r.value = code;
    switch (r.bits.funct3) {
    case 0:
        if (r.bits.funct7 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "addw    %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        } else if (r.bits.funct7 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "mulw    %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        } else if (r.bits.funct7 == 0x20) {
            RISCV_sprintf(tstr, sizeof(tstr), "subw    %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        }
        break;
    case 1:
        RISCV_sprintf(tstr, sizeof(tstr), "sllw    %s,%s,%s",
            RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        break;
    case 4:
        if (r.bits.funct7 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "divw    %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        }
        break;
    case 5:
        if (r.bits.funct7 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "srlw    %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        } else if (r.bits.funct7 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "divuw   %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        } else if (r.bits.funct7 == 0x20) {
            RISCV_sprintf(tstr, sizeof(tstr), "sraw    %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        }
        break;
    case 6:
        if (r.bits.funct7 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "remw    %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        }
        break;
    case 7:
        if (r.bits.funct7 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "remuw   %s,%s,%s",
                RN[r.bits.rd], RN[r.bits.rs1], RN[r.bits.rs2]);
        }
        break;
    default:;
    }
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x14(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_R_type u;
    u.value = code;

    switch (u.bits.funct7) {
    case 0x1:
        RISCV_sprintf(tstr, sizeof(tstr), "fadd.d  %s,%s,%s",
            RF[u.bits.rd], RF[u.bits.rs1], RF[u.bits.rs2]);
        break;
    case 0x5:
        RISCV_sprintf(tstr, sizeof(tstr), "fsub.d  %s,%s,%s",
            RF[u.bits.rd], RF[u.bits.rs1], RF[u.bits.rs2]);
        break;
    case 0x9:
        RISCV_sprintf(tstr, sizeof(tstr), "fmul.d  %s,%s,%s",
            RF[u.bits.rd], RF[u.bits.rs1], RF[u.bits.rs2]);
        break;
    case 0xd:
        RISCV_sprintf(tstr, sizeof(tstr), "fdiv.d  %s,%s,%s",
            RF[u.bits.rd], RF[u.bits.rs1], RF[u.bits.rs2]);
        break;
    case 0x15:
        if (u.bits.funct3 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "fmin.d  %s,%s,%s",
                RF[u.bits.rd], RF[u.bits.rs1], RF[u.bits.rs2]);
        } else if (u.bits.funct3 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "fmax.d  %s,%s,%s",
                RF[u.bits.rd], RF[u.bits.rs1], RF[u.bits.rs2]);
        }
        break;
    case 0x2d:
        if (u.bits.rs2 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "fsqrt.d %s,%s",
                RF[u.bits.rd], RF[u.bits.rs1]);
        }
        break;
    case 0x51:
        if (u.bits.funct3 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "fle.d   %s,%s,%s",
                RN[u.bits.rd], RF[u.bits.rs1], RF[u.bits.rs2]);
        } else if (u.bits.funct3 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "flt.d   %s,%s,%s",
                RN[u.bits.rd], RF[u.bits.rs1], RF[u.bits.rs2]);
        } else if (u.bits.funct3 == 2) {
            RISCV_sprintf(tstr, sizeof(tstr), "feq.d   %s,%s,%s",
                RN[u.bits.rd], RF[u.bits.rs1], RF[u.bits.rs2]);
        }
        break;
    case 0x61:
        if (u.bits.rs2 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "fcvt.w.d  %s,%s",
                RN[u.bits.rd], RF[u.bits.rs1]);
        } else if (u.bits.rs2 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "fcvt.wu.d  %s,%s",
                RN[u.bits.rd], RF[u.bits.rs1]);
        } else if (u.bits.rs2 == 2) {
            RISCV_sprintf(tstr, sizeof(tstr), "fcvt.l.d  %s,%s",
                RN[u.bits.rd], RF[u.bits.rs1]);
        } else if (u.bits.rs2 == 3) {
            RISCV_sprintf(tstr, sizeof(tstr), "fcvt.lu.d  %s,%s",
                RN[u.bits.rd], RF[u.bits.rs1]);
        }
        break;
    case 0x69:
        if (u.bits.rs2 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "fcvt.d.w  %s,%s",
                RF[u.bits.rd], RN[u.bits.rs1]);
        } else if (u.bits.rs2 == 1) {
            RISCV_sprintf(tstr, sizeof(tstr), "fcvt.d.wu %s,%s",
                RF[u.bits.rd], RN[u.bits.rs1]);
        } else if (u.bits.rs2 == 2) {
            RISCV_sprintf(tstr, sizeof(tstr), "fcvt.d.l  %s,%s",
                RF[u.bits.rd], RN[u.bits.rs1]);
        } else if (u.bits.rs2 == 3) {
            RISCV_sprintf(tstr, sizeof(tstr), "fcvt.d.lu %s,%s",
                RF[u.bits.rd], RN[u.bits.rs1]);
        }
        break;
    case 0x71:
        if (u.bits.rs2 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "fmv.x.d %s,%s",
                RN[u.bits.rd], RF[u.bits.rs1]);
        }
        break;
    case 0x79:
        if (u.bits.rs2 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "fmv.d.x %s,%s",
                RF[u.bits.rd], RN[u.bits.rs1]);
        }
        break;
    default:;
    }

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x18(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_SB_type sb;
    uint64_t imm64;

    sb.value = code;
    imm64 = (sb.bits.imm12 << 12) | (sb.bits.imm11 << 11)
                | (sb.bits.imm10_5 << 5) | (sb.bits.imm4_1 << 1);
    if (sb.bits.imm12) {
        imm64 |= EXT_SIGN_12;
    }
    imm64 += pc;
    switch (sb.bits.funct3) {
    case 0:
        if (sb.bits.rs2 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "beqz    %s,%08" RV_PRI64 "x",
                RN[sb.bits.rs1], imm64);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "beq     %s,%s,%08" RV_PRI64 "x",
                RN[sb.bits.rs1], RN[sb.bits.rs2], imm64);
        }
        break;
    case 1:
        if (sb.bits.rs2 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "bnez    %s,%08" RV_PRI64 "x",
                RN[sb.bits.rs1], imm64);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "bne     %s,%s,%08" RV_PRI64 "x",
                RN[sb.bits.rs1], RN[sb.bits.rs2], imm64);
        }
        break;
    case 4:
        if (sb.bits.rs2 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "bltz    %s,%08" RV_PRI64 "x",
                RN[sb.bits.rs1], imm64);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "blt     %s,%s,%08" RV_PRI64 "x",
                RN[sb.bits.rs1], RN[sb.bits.rs2], imm64);
        }
        break;
    case 5:
        if (sb.bits.rs2 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "bgez    %s,%08" RV_PRI64 "x",
                RN[sb.bits.rs1], imm64);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "bge     %s,%s,%08" RV_PRI64 "x",
                RN[sb.bits.rs1], RN[sb.bits.rs2], imm64);
        }
        break;
    case 6:
        if (sb.bits.rs2 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "bltuz   %s,%08" RV_PRI64 "x",
                RN[sb.bits.rs1], imm64);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "bltu    %s,%s,%08" RV_PRI64 "x",
                RN[sb.bits.rs1], RN[sb.bits.rs2], imm64);
        }
        break;
    case 7:
        if (sb.bits.rs2 == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "bgeuz   %s,%08" RV_PRI64 "x",
                RN[sb.bits.rs1], imm64);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "bgeu    %s,%s,%08" RV_PRI64 "x",
                RN[sb.bits.rs1], RN[sb.bits.rs2], imm64);
        }
        break;
    default:;
    }

    /*if (isrc) {
        AttributeType info;
        isrc->addressToSymbol(imm64, &info);
        if (info[0u].size()) {
            if (info[1].to_uint32() == 0) {
                RISCV_sprintf(tcomm, sizeof(tcomm), "%s",
                        info[0u].to_string());
            } else {
                RISCV_sprintf(tcomm, sizeof(tcomm), "%s+%xh",
                        info[0u].to_string(), info[1].to_uint32());
            }
        }
    }*/
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x19(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_I_type i;
    uint64_t imm64;

    i.value = code;
    imm64 = static_cast<int32_t>(code) >> 20;
    if (imm64 == 0) {
        if (i.bits.rs1 == ICpuRiscV::Reg_ra) {
            RISCV_sprintf(tstr, sizeof(tstr), "%s", "ret");
        } else if (i.bits.rd == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "jr      %s",
                RN[i.bits.rs1]);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "jalr    %s",
                RN[i.bits.rs1]);
        }
    } else {
        RISCV_sprintf(tstr, sizeof(tstr), "jalr    %" RV_PRI64 "d,(%s)",
            imm64, RN[i.bits.rs1]);
    }
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x1B(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_UJ_type uj;
    uint64_t imm64;

    uj.value = code;
    imm64 = 0;
    if (uj.bits.imm20) {
        imm64 = 0xfffffffffff00000LL;
    }
    imm64 |= (uj.bits.imm19_12 << 12);
    imm64 |= (uj.bits.imm11 << 11);
    imm64 |= (uj.bits.imm10_1 << 1);
    if (uj.bits.rd) {
        RISCV_sprintf(tstr, sizeof(tstr), "jal     ra,%08" RV_PRI64 "x",
            pc + imm64);
    } else {
        RISCV_sprintf(tstr, sizeof(tstr), "j       %08" RV_PRI64 "x",
            pc + imm64);
    }

    /*if (isrc) {
        AttributeType info;
        isrc->addressToSymbol(pc + imm64, &info);
        if (info[0u].size()) {
            if (info[1].to_uint32() == 0) {
                RISCV_sprintf(tcomm, sizeof(tcomm), "%s",
                        info[0u].to_string());
            } else {
                RISCV_sprintf(tcomm, sizeof(tcomm), "%s+%xh",
                        info[0u].to_string(), info[1].to_uint32());
            }
        }
    }*/
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

int opcode_0x1C(uint64_t pc, uint32_t code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_I_type i;
    uint32_t imm;

    i.value = code;
    imm = static_cast<int32_t>(code) >> 20;

    switch (i.bits.funct3) {
    case 0:
        if (code == 0x00000073) {
            RISCV_sprintf(tstr, sizeof(tstr), "%s", "ecall");
        } else if (code == 0x00100073) {
            RISCV_sprintf(tstr, sizeof(tstr), "%s", "ebreak");
        } else if (code == 0x00200073) {
            RISCV_sprintf(tstr, sizeof(tstr), "%s", "uret");
        } else if (code == 0x10200073) {
            RISCV_sprintf(tstr, sizeof(tstr), "%s", "sret");
        } else if (code == 0x20200073) {
            RISCV_sprintf(tstr, sizeof(tstr), "%s", "hret");
        } else if (code == 0x30200073) {
            RISCV_sprintf(tstr, sizeof(tstr), "%s", "mret");
        }
        break;
    case 1:
        if (i.bits.rd == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "csrw    0x%x,%s",
                imm, RN[i.bits.rs1]);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "csrrw   %s,0x%x,%s",
                RN[i.bits.rd], imm, RN[i.bits.rs1]);
        }
        break;
    case 2:
        if (i.bits.rs1 == 0) {
            // Read
            RISCV_sprintf(tstr, sizeof(tstr), "csrr    %s,0x%x",
                RN[i.bits.rd], imm);
        } else if (i.bits.rd == 0) {
            // Set
            RISCV_sprintf(tstr, sizeof(tstr), "csrs    0x%x,%s",
                imm, RN[i.bits.rs1]);
        } else {
            // Read and set
            RISCV_sprintf(tstr, sizeof(tstr), "csrrs   %s,0x%x,%s",
                RN[i.bits.rd], imm, RN[i.bits.rs1]);
        }
        break;
    case 3:
        if (i.bits.rd == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "csrc    0x%x,%s",
                imm, RN[i.bits.rs1]);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "csrrc   %s,0x%x,%s",
                RN[i.bits.rd], imm, RN[i.bits.rs1]);
        }
        break;
    case 5:
        if (i.bits.rd == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "csrwi   0x%x,0x%x",
                imm, i.bits.rs1);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "csrrwi  %s,0x%x,0x%x",
                RN[i.bits.rd], imm, i.bits.rs1);
        }
        break;
    case 6:
        if (i.bits.rd == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "csrsi   0x%x,0x%x",
                imm, i.bits.rs1);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "csrrsi  %s,0x%x,0x%x",
                RN[i.bits.rd], imm, i.bits.rs1);
        }
        break;
    case 7:
        if (i.bits.rd == 0) {
            RISCV_sprintf(tstr, sizeof(tstr), "csrci   0x%x,0x%x",
                imm, i.bits.rs1);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "csrrci  %s,0x%x,0x%x",
                RN[i.bits.rd], imm, i.bits.rs1);
        }
        break;
    default:;
    }
    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

/**
 * C-extension  disassembler
 */
int C_JR_MV_EBREAK_JALR_ADD(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_CR_type u;
    u.value = code.word;
    if (u.bits.funct4 == 0x8) {
        if (u.bits.rdrs1 && !u.bits.rs2) {
            if (u.bits.rdrs1 == 1) {
                RISCV_sprintf(tstr, sizeof(tstr), "%s", "ret");
            } else {
                RISCV_sprintf(tstr, sizeof(tstr), "jr      %s",
                    RN[u.bits.rdrs1]);
            }
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "mv      %s,%s",
                RN[u.bits.rdrs1], RN[u.bits.rs2]);
        }
    } else {
        if (!u.bits.rdrs1 && !u.bits.rs2) {
            RISCV_sprintf(tstr, sizeof(tstr), "%s", "ebreak");
        } else if (u.bits.rdrs1 && !u.bits.rs2) {
            RISCV_sprintf(tstr, sizeof(tstr), "jalr    ra,%s,0",
                RN[u.bits.rdrs1]);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "add     %s,%s,%s",
                RN[u.bits.rdrs1], RN[u.bits.rdrs1], RN[u.bits.rs2]);
        }
    }

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_NOP_ADDI(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CI_type u;
    u.value = code.word;
    uint64_t imm = u.bits.imm;
    if (u.bits.imm6) {
        imm |= EXT_SIGN_6;
    }
    if (u.value == 0x1) {
        RISCV_sprintf(tstr, sizeof(tstr), "%s", "nop");
    } else {
        RISCV_sprintf(tstr, sizeof(tstr), "addi    %s,%s,%" RV_PRI64 "d",
            RN[u.bits.rdrs], RN[u.bits.rdrs], imm);
    }

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_ADDI16SP_LUI(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CI_type u;
    u.value = code.word;
    if (u.spbits.sp == 0x2) {
        uint64_t imm = (u.spbits.imm8_7 << 3) | (u.spbits.imm6 << 2)
                    | (u.spbits.imm5 << 1) | u.spbits.imm4;
        if (u.spbits.imm9) {
            imm |= EXT_SIGN_6;
        }
        imm <<= 4;
        RISCV_sprintf(tstr, sizeof(tstr), "addi    sp,sp,%" RV_PRI64 "d",
            imm);
    } else {
        uint64_t imm = u.bits.imm;
        if (u.bits.imm6) {
            imm |= EXT_SIGN_6;
        }
        imm <<= 12;
        RISCV_sprintf(tstr, sizeof(tstr), "lui     %s,0x%x",
            RN[u.bits.rdrs], static_cast<uint32_t>(imm));

    }

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_ADDI4SPN(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimp";
    char tcomm[128] = "";
    ISA_CIW_type u;
    u.value = code.word;
    uint64_t imm = (u.bits.imm9_6 << 4) | (u.bits.imm5_4 << 2)
                | (u.bits.imm3 << 1) | u.bits.imm2;
    imm <<= 2;
    if (code.word) {
        RISCV_sprintf(tstr, sizeof(tstr), "addi    %s,sp,%" RV_PRI64 "d",
            RN[8 + u.bits.rd], imm);
    }

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_JAL_ADDIW(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CI_type u;
    u.value = code.word;
    int64_t imm = u.bits.imm;
    if (u.bits.imm6) {
        imm |= EXT_SIGN_6;
    }
    // JAL is th RV32C only instruction
    RISCV_sprintf(tstr, sizeof(tstr), "addiw   %s,%s,%" RV_PRI64 "d",
        RN[u.bits.rdrs], RN[u.bits.rdrs], imm);

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_LD(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CL_type u;
    u.value = code.word;
    uint32_t off = (u.bits.imm27 << 4) | (u.bits.imm6 << 3) | u.bits.imm5_3;
    off <<= 3;

    RISCV_sprintf(tstr, sizeof(tstr), "ld      %s,%d(%s)",
        RN[8 + u.bits.rd], off, RN[8 + u.bits.rs1]);

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_LDSP(uint64_t pc, Reg16Type code,
           AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CI_type u;
    u.value = code.word;
    uint32_t off = (u.ldspbits.off8_6 << 3) | (u.ldspbits.off5 << 2)
                    | u.ldspbits.off4_3;
    off <<= 3;

    RISCV_sprintf(tstr, sizeof(tstr), "ld      %s,%d(sp)",
        RN[u.ldspbits.rd], off);

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_LI(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CI_type u;
    u.value = code.word;
    int64_t imm = u.bits.imm;
    if (u.bits.imm6) {
        imm |= EXT_SIGN_6;
    }

    RISCV_sprintf(tstr, sizeof(tstr), "li      %s,%" RV_PRI64 "d",
        RN[u.bits.rdrs], imm);

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_LW(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CL_type u;
    u.value = code.word;
    uint32_t off = (u.bits.imm6 << 4) | (u.bits.imm5_3 << 1) | u.bits.imm27;
    off <<= 2;

    RISCV_sprintf(tstr, sizeof(tstr), "lw      %s,%d(%s)",
        RN[8 + u.bits.rd], off, RN[8 + u.bits.rs1]);

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_LWSP(uint64_t pc, Reg16Type code,
           AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CI_type u;
    u.value = code.word;
    uint32_t off = (u.lwspbits.off7_6 << 4) | (u.lwspbits.off5 << 3)
                     | u.lwspbits.off4_2;
    off <<= 2;

    RISCV_sprintf(tstr, sizeof(tstr), "lw      %s,%d(sp)",
        RN[u.lwspbits.rd], off);

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_MATH(uint64_t pc, Reg16Type code,
           AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    ISA_CB_type u;
    u.value = code.word;
    uint32_t shamt = (u.shbits.shamt5 << 5) | u.shbits.shamt;
    uint32_t imm = (u.bits.off7_6 << 3) | (u.bits.off2_1 << 1)  | u.bits.off5;

    if (u.bits.off4_3 == 0) {
        RISCV_sprintf(tstr, sizeof(tstr), "srli    %s,%s,%d",
            RN[8 + u.shbits.rd], RN[8 + u.shbits.rd], shamt);
    } else if (u.bits.off4_3 == 1) {
        RISCV_sprintf(tstr, sizeof(tstr), "srai    %s,%s,%d",
            RN[8 + u.shbits.rd], RN[8 + u.shbits.rd], shamt);
    } else if (u.bits.off4_3 == 2) {
        RISCV_sprintf(tstr, sizeof(tstr), "andi    %s,%s,%d",
            RN[8 + u.shbits.rd], RN[8 + u.shbits.rd], imm);
    } else if (u.bits.off8 == 0) {
        ISA_CS_type u2;
        u2.value = code.word;
        switch (u.bits.off7_6) {
        case 0:
            RISCV_sprintf(tstr, sizeof(tstr), "sub     %s,%s,%s",
                RN[8 + u2.bits.rs1], RN[8 + u2.bits.rs1], RN[8 + u2.bits.rs2]);
            break;
        case 1:
            RISCV_sprintf(tstr, sizeof(tstr), "xor     %s,%s,%s",
                RN[8 + u2.bits.rs1], RN[8 + u2.bits.rs1], RN[8 + u2.bits.rs2]);
            break;
        case 2:
            RISCV_sprintf(tstr, sizeof(tstr), "or      %s,%s,%s",
                RN[8 + u2.bits.rs1], RN[8 + u2.bits.rs1], RN[8 + u2.bits.rs2]);
            break;
        default:
            RISCV_sprintf(tstr, sizeof(tstr), "and     %s,%s,%s",
                RN[8 + u2.bits.rs1], RN[8 + u2.bits.rs1], RN[8 + u2.bits.rs2]);
        }
    } else {
        ISA_CS_type u2;
        u2.value = code.word;
        switch (u.bits.off7_6) {
        case 0:
            RISCV_sprintf(tstr, sizeof(tstr), "subw    %s,%s,%s",
                RN[8 + u2.bits.rs1], RN[8 + u2.bits.rs1], RN[8 + u2.bits.rs2]);
            break;
        case 1:
            RISCV_sprintf(tstr, sizeof(tstr), "addw    %s,%s,%s",
                RN[8 + u2.bits.rs1], RN[8 + u2.bits.rs1], RN[8 + u2.bits.rs2]);
            break;
        default:;
        }
    }

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_J(uint64_t pc, Reg16Type code,
        AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CJ_type u;
    u.value = code.word;
    uint64_t off = (u.bits.off10 << 9) | (u.bits.off9_8 << 7)
                    | (u.bits.off7 << 6) | (u.bits.off6 << 5)
                    | (u.bits.off5 << 4) | (u.bits.off4 << 3)
                    | u.bits.off3_1;
    off <<= 1;
    if (u.bits.off11) {
        off |= EXT_SIGN_11;
    }
    RISCV_sprintf(tstr, sizeof(tstr), "j       %" RV_PRI64 "x", pc + off);

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_SD(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CS_type u;
    u.value = code.word;
    uint32_t off = (u.bits.imm27 << 4) | (u.bits.imm6 << 3) | u.bits.imm5_3;
    off <<= 3;

    RISCV_sprintf(tstr, sizeof(tstr), "sd      %s,%d(%s)",
        RN[8 + u.bits.rs2], off, RN[8 + u.bits.rs1]);

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_SDSP(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CSS_type u;
    u.value = code.word;
    uint32_t off = (u.dbits.imm8_6 << 3) | u.dbits.imm5_3;
    off <<= 3;

    RISCV_sprintf(tstr, sizeof(tstr), "sd      %s,%d(sp)",
        RN[u.dbits.rs2], off);

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_SW(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CS_type u;
    u.value = code.word;
    uint32_t off = (u.bits.imm6 << 4) | (u.bits.imm5_3 << 1) | u.bits.imm27;
    off <<= 2;

    RISCV_sprintf(tstr, sizeof(tstr), "sw      %s,%d(%s)",
        RN[8 + u.bits.rs2], off, RN[8 + u.bits.rs1]);

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_SWSP(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CSS_type u;
    u.value = code.word;
    uint32_t off = (u.wbits.imm7_6 << 4) | u.wbits.imm5_2;
    off <<= 2;

    RISCV_sprintf(tstr, sizeof(tstr), "sw      %s,%d(sp)",
        RN[u.dbits.rs2], off);

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_BEQZ(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CB_type u;
    u.value = code.word;
    uint64_t imm = (u.bits.off7_6 << 5) | (u.bits.off5 << 4)
            | (u.bits.off4_3 << 2) | u.bits.off2_1;
    imm <<= 1;
    if (u.bits.off8) {
        imm |= EXT_SIGN_9;
    }

    RISCV_sprintf(tstr, sizeof(tstr), "beqz    %s,%" RV_PRI64 "x",
        RN[8 + u.bits.rs1], pc + imm);

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_BNEZ(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CB_type u;
    u.value = code.word;
    uint64_t imm = (u.bits.off7_6 << 5) | (u.bits.off5 << 4)
            | (u.bits.off4_3 << 2) | u.bits.off2_1;
    imm <<= 1;
    if (u.bits.off8) {
        imm |= EXT_SIGN_9;
    }

    RISCV_sprintf(tstr, sizeof(tstr), "bnez    %s,%" RV_PRI64 "x",
        RN[8 + u.bits.rs1], pc + imm);

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

int C_SLLI(uint64_t pc, Reg16Type code,
                AttributeType *mnemonic, AttributeType *comment) {
    char tstr[128];
    char tcomm[128] = "";
    ISA_CB_type u;
    u.value = code.word;
    uint32_t shamt = (u.shbits.shamt5 << 5) | u.shbits.shamt;
    uint32_t idx = (u.shbits.funct2 << 3) | u.shbits.rd;
    RISCV_sprintf(tstr, sizeof(tstr), "slli    %s,%s,%d",
        RN[idx], RN[idx], shamt);

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 2;
}

}  // namespace debugger
