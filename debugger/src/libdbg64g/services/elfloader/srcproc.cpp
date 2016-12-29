/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Source code processor class declaration.
 */

#include "srcproc.h"
#include <iostream>
#include "riscv-isa.h"

namespace debugger {

/** Class registration in the Core */
REGISTER_CLASS(SourceService)

SourceService::SourceService(const char *name) : IService(name) {
    registerInterface(static_cast<ISourceCode *>(this));
}

SourceService::~SourceService() {
}

void SourceService::postinitService() {
}

int SourceService::disasm(uint64_t pc,
                       AttributeType *data,
                       int offset,
                       AttributeType *mnemonic,
                       AttributeType *comment) {
    char tstr[128] = "unimpl";
    char tcomm[128] = "";
    const char *const *RN = IREGS_NAMES;
    ISA_R_type r;
    ISA_I_type i;
    ISA_U_type u;
    ISA_S_type s;
    ISA_SB_type sb;
    ISA_UJ_type uj;
    int32_t imm;
    uint64_t imm64;
    uint32_t val = (*data)(offset + 3);
    val = (val << 8) | (*data)(offset + 2);
    val = (val << 8) | (*data)(offset + 1);
    val = (val << 8) | (*data)(offset);

    if ((val & 0x3) != 0x3) {
        mnemonic->make_string(tstr);
        comment->make_string(tcomm);
        return 4;
    }

    uint32_t opcode1 = (val >> 2) & 0x1f;
    uint32_t opcode2 = (val >> 12) & 0x7;
    switch (opcode1) {
    case 0x0:
        i.value = val;
        imm = static_cast<int32_t>(val) >> 20;
        switch (opcode2) {
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
        break;
    case 0x8:
        s.value = val;
        imm = (s.bits.imm11_5 << 5) | s.bits.imm4_0;
        if (imm & 0x800) {
            imm |= EXT_SIGN_12;
        }
        switch (opcode2) {
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
        break;
    case 0xC:
        r.value = val;
        switch (opcode2) {
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
        break;
    case 0x4:
        i.value = val;
        imm = static_cast<int32_t>(val) >> 20;
        switch (opcode2) {
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
            if ((val >> 26) == 0) {
                RISCV_sprintf(tstr, sizeof(tstr), "srli    %s,%s,%d",
                    RN[i.bits.rd], RN[i.bits.rs1], imm);
            } else if ((val >> 26) == 0x20) {
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
        break;
    case 0x6:
        i.value = val;
        imm = static_cast<int32_t>(val) >> 20;
        switch (opcode2) {
        case 0:
            RISCV_sprintf(tstr, sizeof(tstr), "addiw   %s,%s,%d",
                RN[i.bits.rd], RN[i.bits.rs1], imm);
            break;
        case 1:
            RISCV_sprintf(tstr, sizeof(tstr), "slliw   %s,%s,%d",
                RN[i.bits.rd], RN[i.bits.rs1], imm);
            break;
        case 5:
            if (r.bits.funct7 == 0) {
                RISCV_sprintf(tstr, sizeof(tstr), "srliw   %s,%s,%d",
                    RN[i.bits.rd], RN[i.bits.rs1], imm);
            } else if (r.bits.funct7 == 0x20) {
                RISCV_sprintf(tstr, sizeof(tstr), "sraiw   %s,%s,%d",
                    RN[i.bits.rd], RN[i.bits.rs1], imm);
            }
            break;
        default:;
        }
        break;
    case 0xE:
        r.value = val;
        switch (opcode2) {
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
        break;
    case 0x5:
        u.value = val;
        imm64 = u.bits.imm31_12 << 12;
        if (imm64 & (1LL << 31)) {
            imm64 |= EXT_SIGN_32;
        }
        RISCV_sprintf(tstr, sizeof(tstr), "auipc   %s,%" RV_PRI64 "x",
            RN[u.bits.rd], imm64);
        break;
    case 0x18:
        sb.value = val;
        imm64 = (sb.bits.imm12 << 12) | (sb.bits.imm11 << 11)
                    | (sb.bits.imm10_5 << 5) | (sb.bits.imm4_1 << 1);
        if (sb.bits.imm12) {
            imm64 |= EXT_SIGN_12;
        }
        break;
        switch (opcode2) {
        case 0:
            RISCV_sprintf(tstr, sizeof(tstr), "beq     %s,%s,%08" RV_PRI64 "x",
                RN[sb.bits.rs2], RN[sb.bits.rs1], imm64);
            break;
        case 1:
            RISCV_sprintf(tstr, sizeof(tstr), "bne     %s,%s,%08" RV_PRI64 "x",
                RN[sb.bits.rs2], RN[sb.bits.rs1], imm64);
            break;
        case 4:
            RISCV_sprintf(tstr, sizeof(tstr), "blt     %s,%s,%08" RV_PRI64 "x",
                RN[sb.bits.rs2], RN[sb.bits.rs1], imm64);
            break;
        case 5:
            RISCV_sprintf(tstr, sizeof(tstr), "bge     %s,%s,%08" RV_PRI64 "x",
                RN[sb.bits.rs2], RN[sb.bits.rs1], imm64);
            break;
        case 6:
            RISCV_sprintf(tstr, sizeof(tstr), "bltu    %s,%s,%08" RV_PRI64 "x",
                RN[sb.bits.rs2], RN[sb.bits.rs1], imm64);
            break;
        case 7:
            RISCV_sprintf(tstr, sizeof(tstr), "bgeu    %s,%s,%08" RV_PRI64 "x",
                RN[sb.bits.rs2], RN[sb.bits.rs1], imm64);
            break;
        default:;
        }
        break;
    case 0x19:
        i.value = val;
        imm64 = static_cast<int32_t>(val) >> 20;
        if (imm64 == 0) {
            if (i.bits.rs1 == Reg_ra) {
                RISCV_sprintf(tstr, sizeof(tstr), "%s", "ret");
            } else {
                RISCV_sprintf(tstr, sizeof(tstr), "jalr    %s",
                    RN[i.bits.rs1]);
            }
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "jalr    %s,%08" RV_PRI64 "x",
                RN[i.bits.rs1], pc + imm64);
        }
        break;
    case 0x1B:
        uj.value = val;
        imm64 = 0;
        if (uj.bits.imm20) {
            imm64 = 0xfffffffffff00000LL;
        }
        imm64 |= (uj.bits.imm19_12 << 12);
        imm64 |= (uj.bits.imm11 << 11);
        imm64 |= (uj.bits.imm10_1 << 1);
        if (uj.bits.rd) {
            RISCV_sprintf(tstr, sizeof(tstr), "jal     %08" RV_PRI64 "x",
                pc + imm64);
        } else {
            RISCV_sprintf(tstr, sizeof(tstr), "j       %08" RV_PRI64 "x",
                pc + imm64);
        }
        break;
    default:;
    }

    mnemonic->make_string(tstr);
    comment->make_string(tcomm);
    return 4;
}

}  // namespace debugger
