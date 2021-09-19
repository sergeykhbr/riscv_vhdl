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

#include "api_types.h"
#include "api_core.h"
#include "riscv_disasm.h"

namespace debugger {

int riscv_disassembler(uint32_t instr, char *disasm, size_t strsz) {
    if ((instr & 0x3) != 3) {
        uint32_t op1 = (instr >> 13) & 0x7;
        uint32_t i12 = (instr >> 12) & 0x1;
        switch (instr & 0x3) {
        case 0:
            switch (op1) {
            case 0:
                if (((instr >> 2) & 0x7FF) == 0) {
                    RISCV_sprintf(disasm, strsz, "%10s", "ERROR");
                } else {
                    RISCV_sprintf(disasm, strsz, "%10s", "c.addi4spn");
                }
                break;
            case 1:
                RISCV_sprintf(disasm, strsz, "%10s", "c.fld");
                break;
            case 2:
                RISCV_sprintf(disasm, strsz, "%10s", "c.lw");
                break;
            case 3:
                RISCV_sprintf(disasm, strsz, "%10s", "c.ld");
                break;
            case 4:
                RISCV_sprintf(disasm, strsz, "%10s", "ERROR");
                break;
            case 5:
                RISCV_sprintf(disasm, strsz, "%10s", "c.fsd");
                break;
            case 6:
                RISCV_sprintf(disasm, strsz, "%10s", "c.sw");
                break;
            case 7:
                RISCV_sprintf(disasm, strsz, "%10s", "c.sd");
                break;
            default:;
            }
            break;
        case 1:
            switch (op1) {
            case 0:
                if (((instr >> 2) & 0x7FF) == 0) {
                    RISCV_sprintf(disasm, strsz, "%10s", "c.nop");
                } else {
                    RISCV_sprintf(disasm, strsz, "%10s", "c.addi");
                }
                break;
            case 1:
                if (((instr >> 7) & 0x1F) == 0) {
                    RISCV_sprintf(disasm, strsz, "%10s", "ERROR");
                } else {
                    RISCV_sprintf(disasm, strsz, "%10s", "c.addiw");
                }
                break;
            case 2:
                if (((instr >> 7) & 0x1F) == 0) {
                    RISCV_sprintf(disasm, strsz, "%10s", "ERROR");
                } else {
                    RISCV_sprintf(disasm, strsz, "%10s", "c.li");
                }
                break;
            case 3:
                if (((instr >> 7) & 0x1F) == 2) {
                    RISCV_sprintf(disasm, strsz, "%10s", "c.addi16sp");
                } else if (((instr >> 7) & 0x1F) != 0) {
                    RISCV_sprintf(disasm, strsz, "%10s", "c.lui");
                } else {
                    RISCV_sprintf(disasm, strsz, "%10s", "ERROR");
                }
                break;
            case 4:
                if (((instr >> 10) & 0x3) == 0) {
                    if (i12 == 0 && ((instr >> 2) & 0x1f) == 0) {
                        RISCV_sprintf(disasm, strsz, "%10s", "c.srli64");
                    } else {
                        RISCV_sprintf(disasm, strsz, "%10s", "c.srli");
                    }
                } else if (((instr >> 10) & 0x3) == 1) {
                    if (i12 == 0 && ((instr >> 2) & 0x1f) == 0) {
                        RISCV_sprintf(disasm, strsz, "%10s", "c.srai64");
                    } else {
                        RISCV_sprintf(disasm, strsz, "%10s", "c.srai");
                    }
                } else if (((instr >> 10) & 0x3) == 2) {
                    RISCV_sprintf(disasm, strsz, "%10s", "c.andi");
                } else {
                    if (i12 == 0 && ((instr >> 5) & 0x3) == 0) {
                        RISCV_sprintf(disasm, strsz, "%10s", "c.sub");
                    } else if (i12 == 0 && ((instr >> 5) & 0x3) == 1) {
                        RISCV_sprintf(disasm, strsz, "%10s", "c.xor");
                    } else if (i12 == 0 && ((instr >> 5) & 0x3) == 2) {
                        RISCV_sprintf(disasm, strsz, "%10s", "c.or");
                    } else if (i12 == 0 && ((instr >> 5) & 0x3) == 3) {
                        RISCV_sprintf(disasm, strsz, "%10s", "c.and");
                    } else if (i12 == 1 && ((instr >> 5) & 0x3) == 0) {
                        RISCV_sprintf(disasm, strsz, "%10s", "c.subw");
                    } else if (i12 == 1 && ((instr >> 5) & 0x3) == 1) {
                        RISCV_sprintf(disasm, strsz, "%10s", "c.addw");
                    } else {
                        RISCV_sprintf(disasm, strsz, "%10s", "ERROR");
                    }
                }
                break;
            case 5:
                RISCV_sprintf(disasm, strsz, "%10s", "c.j");
                break;
            case 6:
                RISCV_sprintf(disasm, strsz, "%10s", "c.beqz");
                break;
            case 7:
                RISCV_sprintf(disasm, strsz, "%10s", "c.bnez");
                break;
            default:;
            }
            break;
        case 2:
            switch (op1) {
            case 0:
                if (i12 == 0 && ((instr >> 5) & 0x3) == 0) {
                    RISCV_sprintf(disasm, strsz, "%10s", "c.slli64");
                } else {
                    RISCV_sprintf(disasm, strsz, "%10s", "c.slli");
                }
                break;
            case 1:
                RISCV_sprintf(disasm, strsz, "%10s", "c.fldsp");
                break;
            case 2:
                RISCV_sprintf(disasm, strsz, "%10s", "c.lwsp");
                break;
            case 3:
                RISCV_sprintf(disasm, strsz, "%10s", "c.ldsp");
                break;
            case 4:
                if (i12 == 0 && ((instr >> 2) & 0x1f) == 0) {
                    RISCV_sprintf(disasm, strsz, "%10s", "c.jr");
                } else if (i12 == 0 && ((instr >> 2) & 0x1f) != 0) {
                    RISCV_sprintf(disasm, strsz, "%10s", "c.mv");
                } else if (i12 == 1 && ((instr >> 2) & 0x1f) == 0
                    && ((instr >> 7) & 0x1f) == 0) {
                    RISCV_sprintf(disasm, strsz, "%10s", "c.ebreak");
                } else if (i12 == 1 && ((instr >> 2) & 0x1f) == 0) {
                    RISCV_sprintf(disasm, strsz, "%10s", "c.jalr");
                } else {
                    RISCV_sprintf(disasm, strsz, "%10s", "c.add");
                }
                break;
            case 5:
                RISCV_sprintf(disasm, strsz, "%10s", "c.fsdsp");
                break;
            case 6:
                RISCV_sprintf(disasm, strsz, "%10s", "c.swsp");
                break;
            case 7:
                RISCV_sprintf(disasm, strsz, "%10s", "c.sdsp");
                break;
            default:;
            }
            break;
        case 3:
            break;
        default:;
        }
    } else {
        uint32_t op1 = (instr >> 2) & 0x1f;
        uint32_t op2 = (instr >> 12) & 0x7;
        switch (op1) {
        case 0:
            switch (op2) {
            case 0: RISCV_sprintf(disasm, strsz, "%10s", "lb"); break;
            case 1: RISCV_sprintf(disasm, strsz, "%10s", "lh"); break;
            case 2: RISCV_sprintf(disasm, strsz, "%10s", "lw"); break;
            case 3: RISCV_sprintf(disasm, strsz, "%10s", "ld"); break;
            case 4: RISCV_sprintf(disasm, strsz, "%10s", "lbu"); break;
            case 5: RISCV_sprintf(disasm, strsz, "%10s", "lhu"); break;
            case 6: RISCV_sprintf(disasm, strsz, "%10s", "lwu"); break;
            default: RISCV_sprintf(disasm, strsz, "%10s", "error");
            }
            break;
        case 0x04:
            switch (op2) {
            case 0: RISCV_sprintf(disasm, strsz, "%10s", "addi"); break;
            case 1: RISCV_sprintf(disasm, strsz, "%10s", "slli"); break;
            case 2: RISCV_sprintf(disasm, strsz, "%10s", "slti"); break;
            case 3: RISCV_sprintf(disasm, strsz, "%10s", "sltiu"); break;
            case 4: RISCV_sprintf(disasm, strsz, "%10s", "xori"); break;
            case 5:
                if (((instr >> 26) & 0x3f) == 0) {
                    RISCV_sprintf(disasm, strsz, "%10s", "srli");
                } else if (((instr >> 26) & 0x3f) == 0x10) {
                    RISCV_sprintf(disasm, strsz, "%10s", "srai");
                } else {
                    RISCV_sprintf(disasm, strsz, "%10s", "error");
                }
                break;
            case 6: RISCV_sprintf(disasm, strsz, "%10s", "ori"); break;
            case 7: RISCV_sprintf(disasm, strsz, "%10s", "andi"); break;
            default: RISCV_sprintf(disasm, strsz, "%10s", "error");
            }
        break;
        case 0x05:
            RISCV_sprintf(disasm, strsz, "%10s", "auipc");
            break;
        case 0x06:
            switch (op2) {
            case 0: RISCV_sprintf(disasm, strsz, "%10s", "addiw"); break;
            case 1: RISCV_sprintf(disasm, strsz, "%10s", "slliw"); break;
            case 5:
                if (((instr >> 25) & 0x7f) == 0x00) {
                    RISCV_sprintf(disasm, strsz, "%10s", "srliw");
                } else if (((instr >> 25) & 0x7f) == 0x20) {
                    RISCV_sprintf(disasm, strsz, "%10s", "sraiw");
                }
                break;
            default: RISCV_sprintf(disasm, strsz, "%10s", "error");
            }
            break;
        case 0x08:
            switch (op2) {
            case 0: RISCV_sprintf(disasm, strsz, "%10s", "sb"); break;
            case 1: RISCV_sprintf(disasm, strsz, "%10s", "sh"); break;
            case 2: RISCV_sprintf(disasm, strsz, "%10s", "sw"); break;
            case 3: RISCV_sprintf(disasm, strsz, "%10s", "sd"); break;
            default: RISCV_sprintf(disasm, strsz, "%10s", "error");
            }
            break;
        case 0x0B:
            if (op2 == 2) {
                switch ((instr >> 27) & 0x1F) {
                case 0x0: RISCV_sprintf(disasm, strsz, "%10s", "amoadd.w"); break;
                case 0x1: RISCV_sprintf(disasm, strsz, "%10s", "amoswap.w"); break;
                case 0x2:
                    if (((instr >> 20) & 0x1f) == 0) {
                        RISCV_sprintf(disasm, strsz, "%10s", "lr.w");
                    } else {
                        RISCV_sprintf(disasm, strsz, "%10s", "error");
                    }
                    break;
                case 0x3: RISCV_sprintf(disasm, strsz, "%10s", "sc.w"); break;
                case 0x4: RISCV_sprintf(disasm, strsz, "%10s", "amoxor.w"); break;
                case 0x8: RISCV_sprintf(disasm, strsz, "%10s", "amoor.w"); break;
                case 0xc: RISCV_sprintf(disasm, strsz, "%10s", "amoand.w"); break;
                case 0x10: RISCV_sprintf(disasm, strsz, "%10s", "amomin.w"); break;
                case 0x14: RISCV_sprintf(disasm, strsz, "%10s", "amomax.w"); break;
                case 0x18: RISCV_sprintf(disasm, strsz, "%10s", "amominu.w"); break;
                case 0x1c: RISCV_sprintf(disasm, strsz, "%10s", "amomaxu.w"); break;
                default: RISCV_sprintf(disasm, strsz, "%10s", "error");
                }
            } else if (op2 == 3) {
                switch ((instr >> 27) & 0x1F) {
                case 0x0: RISCV_sprintf(disasm, strsz, "%10s", "amoadd.d"); break;
                case 0x1: RISCV_sprintf(disasm, strsz, "%10s", "amoswap.d"); break;
                case 0x2:
                    if (((instr >> 20) & 0x1f) == 0) {
                        RISCV_sprintf(disasm, strsz, "%10s", "lr.d");
                    } else {
                        RISCV_sprintf(disasm, strsz, "%10s", "error.d");
                    }
                    break;
                case 0x3: RISCV_sprintf(disasm, strsz, "%10s", "sc.d"); break;
                case 0x4: RISCV_sprintf(disasm, strsz, "%10s", "amoxor.d"); break;
                case 0x8: RISCV_sprintf(disasm, strsz, "%10s", "amoor.d"); break;
                case 0xc: RISCV_sprintf(disasm, strsz, "%10s", "amoand.d"); break;
                case 0x10: RISCV_sprintf(disasm, strsz, "%10s", "amomin.d"); break;
                case 0x14: RISCV_sprintf(disasm, strsz, "%10s", "amomax.d"); break;
                case 0x18: RISCV_sprintf(disasm, strsz, "%10s", "amominu.d"); break;
                case 0x1c: RISCV_sprintf(disasm, strsz, "%10s", "amomaxu.d"); break;
                default: RISCV_sprintf(disasm, strsz, "%10s", "error");
                }
            } else {
                RISCV_sprintf(disasm, strsz, "%10s", "error");
            }
        break;
        case 0x0C:
            switch (op2) {
            case 0:
                if (((instr >> 25) & 0x7f) == 0x00) {
                    RISCV_sprintf(disasm, strsz, "%10s", "add");
                } else if (((instr >> 25) & 0x7f) == 0x01) {
                    RISCV_sprintf(disasm, strsz, "%10s", "mul");
                } else if (((instr >> 25) & 0x7f) == 0x20) {
                    RISCV_sprintf(disasm, strsz, "%10s", "sub");
                } else {
                    RISCV_sprintf(disasm, strsz, "%10s", "error");
                }
                break;
            case 1:
                if (((instr >> 25) & 0x7f) == 0x00) {
                    RISCV_sprintf(disasm, strsz, "%10s", "sll");
                } else if (((instr >> 25) & 0x7f) == 0x01) {
                    RISCV_sprintf(disasm, strsz, "%10s", "mulh");
                } else {
                    RISCV_sprintf(disasm, strsz, "%10s", "error");
                }
                break;
            case 2:
                if (((instr >> 25) & 0x7f) == 0x00) {
                    RISCV_sprintf(disasm, strsz, "%10s", "slt");
                } else if (((instr >> 25) & 0x7f) == 0x01) {
                    RISCV_sprintf(disasm, strsz, "%10s", "mulhsu");
                } else {
                    RISCV_sprintf(disasm, strsz, "%10s", "error");
                }
                break;
            case 3:
                if (((instr >> 25) & 0x7f) == 0x00) {
                    RISCV_sprintf(disasm, strsz, "%10s", "sltu");
                } else if (((instr >> 25) & 0x7f) == 0x01) {
                    RISCV_sprintf(disasm, strsz, "%10s", "mulhu");
                } else {
                    RISCV_sprintf(disasm, strsz, "%10s", "error");
                }
                break;
            case 4:
                if (((instr >> 25) & 0x7f) == 0x00) {
                    RISCV_sprintf(disasm, strsz, "%10s", "xor");
                } else if (((instr >> 25) & 0x7f) == 0x01) {
                    RISCV_sprintf(disasm, strsz, "%10s", "div");
                } else {
                    RISCV_sprintf(disasm, strsz, "%10s", "error");
                }
                break;
            case 5:
                if (((instr >> 25) & 0x7f) == 0x00) {
                    RISCV_sprintf(disasm, strsz, "%10s", "srl");
                } else if (((instr >> 25) & 0x7f) == 0x01) {
                    RISCV_sprintf(disasm, strsz, "%10s", "divu");
                } else if (((instr >> 25) & 0x7f) == 0x20) {
                    RISCV_sprintf(disasm, strsz, "%10s", "sra");
                } else {
                    RISCV_sprintf(disasm, strsz, "%10s", "error");
                }
                break;
            case 6:
                if (((instr >> 25) & 0x7f) == 0x00) {
                    RISCV_sprintf(disasm, strsz, "%10s", "or");
                } else if (((instr >> 25) & 0x7f) == 0x01) {
                    RISCV_sprintf(disasm, strsz, "%10s", "rem");
                } else {
                    RISCV_sprintf(disasm, strsz, "%10s", "error");
                }
                break;
            case 7:
                if (((instr >> 25) & 0x7f) == 0x00) {
                    RISCV_sprintf(disasm, strsz, "%10s", "and");
                } else if (((instr >> 25) & 0x7f) == 0x01) {
                    RISCV_sprintf(disasm, strsz, "%10s", "remu");
                } else {
                    RISCV_sprintf(disasm, strsz, "%10s", "error");
                }
                break;
            default:
                RISCV_sprintf(disasm, strsz, "%10s", "unknown");
            }
            break;
        case 0x0D:
            RISCV_sprintf(disasm, strsz, "%10s", "lui");
            break;
        case 0x0E:
            switch (op2) {
            case 0:
                if (((instr >> 25) & 0x7f) == 0x00) {
                    RISCV_sprintf(disasm, strsz, "%10s", "addw");
                } else if (((instr >> 25) & 0x7f) == 0x01) {
                    RISCV_sprintf(disasm, strsz, "%10s", "mulw");
                } else if (((instr >> 25) & 0x7f) == 0x20) {
                    RISCV_sprintf(disasm, strsz, "%10s", "subw");
                } else {
                    RISCV_sprintf(disasm, strsz, "%10s", "error");
                }
                break;
            case 1:
                RISCV_sprintf(disasm, strsz, "%10s", "sllw");
                break;
            case 4:
                if (((instr >> 25) & 0x7f) == 0x01) {
                    RISCV_sprintf(disasm, strsz, "%10s", "divw");
                } else {
                    RISCV_sprintf(disasm, strsz, "%10s", "error");
                }
                break;
            case 5:
                if (((instr >> 25) & 0x7f) == 0x00) {
                    RISCV_sprintf(disasm, strsz, "%10s", "srlw");
                } else if (((instr >> 25) & 0x7f) == 0x01) {
                    RISCV_sprintf(disasm, strsz, "%10s", "divuw");
                } else if (((instr >> 25) & 0x7f) == 0x20) {
                    RISCV_sprintf(disasm, strsz, "%10s", "sraw");
                } else {
                    RISCV_sprintf(disasm, strsz, "%10s", "error");
                }
                break;
            case 6:
                if (((instr >> 25) & 0x7f) == 0x01) {
                    RISCV_sprintf(disasm, strsz, "%10s", "remw");
                } else {
                    RISCV_sprintf(disasm, strsz, "%10s", "error");
                }
                break;
            case 7:
                if (((instr >> 25) & 0x7f) == 0x01) {
                    RISCV_sprintf(disasm, strsz, "%10s", "remuw");
                } else {
                    RISCV_sprintf(disasm, strsz, "%10s", "error");
                }
                break;
            default: RISCV_sprintf(disasm, strsz, "%10s", "error");
            }
            break;
        case 0x18:
            switch (op2) {
            case 0: RISCV_sprintf(disasm, strsz, "%10s", "beq"); break;
            case 1: RISCV_sprintf(disasm, strsz, "%10s", "bne"); break;
            case 4: RISCV_sprintf(disasm, strsz, "%10s", "blt"); break;
            case 5: RISCV_sprintf(disasm, strsz, "%10s", "bge"); break;
            case 6: RISCV_sprintf(disasm, strsz, "%10s", "bltu"); break;
            case 7: RISCV_sprintf(disasm, strsz, "%10s", "bgeu"); break;
            default: RISCV_sprintf(disasm, strsz, "%10s", "error");
            }
            break;
        case 0x19:
            RISCV_sprintf(disasm, strsz, "%10s", "jalr");
            break;
        case 0x1B :
            RISCV_sprintf(disasm, strsz, "%10s", "jal");
            break;
        case 0x1C:
            switch (op2) {
            case 0:
                if (instr == 0x00000073) {
                    RISCV_sprintf(disasm, strsz, "%10s", "ecall");
                } else if (instr == 0x00100073) {
                    RISCV_sprintf(disasm, strsz, "%10s", "ebreak");
                } else if (instr == 0x00200073) {
                    RISCV_sprintf(disasm, strsz, "%10s", "uret");
                } else if (instr == 0x10200073) {
                    RISCV_sprintf(disasm, strsz, "%10s", "sret");
                } else if (instr == 0x20200073) {
                    RISCV_sprintf(disasm, strsz, "%10s", "hret");
                } else if (instr == 0x30200073) {
                    RISCV_sprintf(disasm, strsz, "%10s", "mret");
                } else {
                    RISCV_sprintf(disasm, strsz, "%10s", "error");
                }
                break;
            case 1:
                RISCV_sprintf(disasm, strsz, "%10s", "csrrw");
                break;
            case 2:
                RISCV_sprintf(disasm, strsz, "%10s", "csrrs");
                break;
            case 3:
                RISCV_sprintf(disasm, strsz, "%10s", "csrrc");
                break;
            case 5:
                RISCV_sprintf(disasm, strsz, "%10s", "csrrwi");
                break;
            case 6:
                RISCV_sprintf(disasm, strsz, "%10s", "csrrsi");
                break;
            case 7:
                RISCV_sprintf(disasm, strsz, "%10s", "csrrci");
                break;
            default:
                RISCV_sprintf(disasm, strsz, "%10s", "error");
            }
            break;
        default:
            RISCV_sprintf(disasm, strsz, "%10s", "unknown");
        }
    }
    return 0;
}

}  // namespace debugger
