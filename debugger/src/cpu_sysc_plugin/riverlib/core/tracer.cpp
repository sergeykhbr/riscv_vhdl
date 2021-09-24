/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
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
 *
 */

#include "api_types.h"
#include "api_core.h"
#include "tracer.h"

namespace debugger {

static const char *rname[] = {
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6",
    "ft0", "ft1", "ft2",  "ft3",  "ft4", "ft5", "ft6",  "ft7",
    "fs0", "fs1", "fa0",  "fa1",  "fa2", "fa3", "fa4",  "fa5",
    "fa6", "fa7", "fs2",  "fs3",  "fs4", "fs5", "fs6",  "fs7",
    "fs8", "fs9", "fs10", "fs11", "ft8", "ft9", "ft10", "ft11"
};

Tracer::Tracer(sc_module_name name_, bool async_reset, const char *trace_file)
    : sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_dbg_executed_cnt("i_dbg_executed_cnt"),
    i_e_valid("i_e_valid"),
    i_e_pc("i_e_pc"),
    i_e_instr("i_e_instr"),
    i_e_wena("i_e_wena"),
    i_e_waddr("i_e_waddr"),
    i_e_wdata("i_e_wdata"),
    i_e_memop_valid("i_e_memop_valid"),
    i_e_memop_type("i_e_memop_type"),
    i_e_memop_size("i_e_memop_size"),
    i_e_memop_addr("i_e_memop_addr"),
    i_e_memop_wdata("i_e_memop_wdata"),
    i_m_memop_ready("i_m_memop_ready"),
    i_m_wena("i_m_wena"),
    i_m_waddr("i_m_waddr"),
    i_m_wdata("i_m_wdata"),
    i_reg_ignored("i_reg_ignored") {
    async_reset_ = async_reset;
    fl_ = 0;
    if (strlen(trace_file)) {
        fl_ = fopen(trace_file, "wb");
    }

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_dbg_executed_cnt;
    sensitive << i_e_valid;
    sensitive << i_e_pc;
    sensitive << i_e_instr;
    sensitive << i_e_wena;
    sensitive << i_e_waddr;
    sensitive << i_e_wdata;
    sensitive << i_e_memop_valid;
    sensitive << i_e_memop_type;
    sensitive << i_e_memop_size;
    sensitive << i_e_memop_addr;
    sensitive << i_e_memop_wdata;
    sensitive << i_m_memop_ready;
    sensitive << i_m_wena;
    sensitive << i_m_waddr;
    sensitive << i_m_wdata;
    sensitive << i_reg_ignored;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    tr_total_ = 0;
    tr_wcnt_ = 0;
    tr_rcnt_ = 0;
    memset(&trace_tbl_, 0, sizeof(trace_tbl_));
};

void Tracer::comb() {

}

void Tracer::task_disassembler(uint32_t instr) {
    if ((instr & 0x3) != 3) {
        uint32_t op1 = (instr >> 13) & 0x7;
        uint32_t i12 = (instr >> 12) & 0x1;
        size_t strsz = sizeof(disasm);
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
            case 0: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "lb"); break;
            case 1: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "lh"); break;
            case 2: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "lw"); break;
            case 3: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "ld"); break;
            case 4: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "lbu"); break;
            case 5: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "lhu"); break;
            case 6: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "lwu"); break;
            default: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "error");
            }
            break;
        case 0x04:
            switch (op2) {
            case 0: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "addi"); break;
            case 1: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "slli"); break;
            case 2: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "slti"); break;
            case 3: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "sltiu"); break;
            case 4: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "xori"); break;
            case 5:
                if (((instr >> 26) & 0x3f) == 0) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "srli");
                } else if (((instr >> 26) & 0x3f) == 0x10) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "srai");
                } else {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "error");
                }
                break;
            case 6: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "ori"); break;
            case 7: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "andi"); break;
            default: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "error");
            }
        break;
        case 0x05:
            RISCV_sprintf(disasm, sizeof(disasm), "%10s", "auipc");
            break;
        case 0x06:
            switch (op2) {
            case 0: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "addiw"); break;
            case 1: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "slliw"); break;
            case 5:
                if (((instr >> 25) & 0x7f) == 0x00) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "srliw");
                } else if (((instr >> 25) & 0x7f) == 0x20) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "sraiw");
                }
                break;
            default: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "error");
            }
            break;
        case 0x08:
            switch (op2) {
            case 0: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "sb"); break;
            case 1: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "sh"); break;
            case 2: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "sw"); break;
            case 3: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "sd"); break;
            default: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "error");
            }
            break;
        case 0x0B:
            if (op2 == 2) {
                switch ((instr >> 27) & 0x1F) {
                case 0x0: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "amoadd.w"); break;
                case 0x1: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "amoswap.w"); break;
                case 0x2:
                    if (((instr >> 20) & 0x1f) == 0) {
                        RISCV_sprintf(disasm, sizeof(disasm), "%10s", "lr.w");
                    } else {
                        RISCV_sprintf(disasm, sizeof(disasm), "%10s", "error");
                    }
                    break;
                case 0x3: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "sc.w"); break;
                case 0x4: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "amoxor.w"); break;
                case 0x8: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "amoor.w"); break;
                case 0xc: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "amoand.w"); break;
                case 0x10: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "amomin.w"); break;
                case 0x14: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "amomax.w"); break;
                case 0x18: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "amominu.w"); break;
                case 0x1c: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "amomaxu.w"); break;
                default: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "error");
                }
            } else if (op2 == 3) {
                switch ((instr >> 27) & 0x1F) {
                case 0x0: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "amoadd.d"); break;
                case 0x1: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "amoswap.d"); break;
                case 0x2:
                    if (((instr >> 20) & 0x1f) == 0) {
                        RISCV_sprintf(disasm, sizeof(disasm), "%10s", "lr.d");
                    } else {
                        RISCV_sprintf(disasm, sizeof(disasm), "%10s", "error.d");
                    }
                    break;
                case 0x3: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "sc.d"); break;
                case 0x4: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "amoxor.d"); break;
                case 0x8: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "amoor.d"); break;
                case 0xc: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "amoand.d"); break;
                case 0x10: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "amomin.d"); break;
                case 0x14: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "amomax.d"); break;
                case 0x18: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "amominu.d"); break;
                case 0x1c: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "amomaxu.d"); break;
                default: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "error");
                }
            } else {
                RISCV_sprintf(disasm, sizeof(disasm), "%10s", "error");
            }
        break;
        case 0x0C:
            switch (op2) {
            case 0:
                if (((instr >> 25) & 0x7f) == 0x00) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "add");
                } else if (((instr >> 25) & 0x7f) == 0x01) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "mul");
                } else if (((instr >> 25) & 0x7f) == 0x20) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "sub");
                } else {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "error");
                }
                break;
            case 1:
                if (((instr >> 25) & 0x7f) == 0x00) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "sll");
                } else if (((instr >> 25) & 0x7f) == 0x01) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "mulh");
                } else {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "error");
                }
                break;
            case 2:
                if (((instr >> 25) & 0x7f) == 0x00) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "slt");
                } else if (((instr >> 25) & 0x7f) == 0x01) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "mulhsu");
                } else {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "error");
                }
                break;
            case 3:
                if (((instr >> 25) & 0x7f) == 0x00) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "sltu");
                } else if (((instr >> 25) & 0x7f) == 0x01) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "mulhu");
                } else {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "error");
                }
                break;
            case 4:
                if (((instr >> 25) & 0x7f) == 0x00) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "xor");
                } else if (((instr >> 25) & 0x7f) == 0x01) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "div");
                } else {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "error");
                }
                break;
            case 5:
                if (((instr >> 25) & 0x7f) == 0x00) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "srl");
                } else if (((instr >> 25) & 0x7f) == 0x01) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "divu");
                } else if (((instr >> 25) & 0x7f) == 0x20) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "sra");
                } else {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "error");
                }
                break;
            case 6:
                if (((instr >> 25) & 0x7f) == 0x00) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "or");
                } else if (((instr >> 25) & 0x7f) == 0x01) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "rem");
                } else {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "error");
                }
                break;
            case 7:
                if (((instr >> 25) & 0x7f) == 0x00) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "and");
                } else if (((instr >> 25) & 0x7f) == 0x01) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "remu");
                } else {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "error");
                }
                break;
            default:
                RISCV_sprintf(disasm, sizeof(disasm), "%10s", "unknown");
            }
            break;
        case 0x0D:
            RISCV_sprintf(disasm, sizeof(disasm), "%10s", "lui");
            break;
        case 0x0E:
            switch (op2) {
            case 0:
                if (((instr >> 25) & 0x7f) == 0x00) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "addw");
                } else if (((instr >> 25) & 0x7f) == 0x01) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "mulw");
                } else if (((instr >> 25) & 0x7f) == 0x20) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "subw");
                } else {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "error");
                }
                break;
            case 1:
                RISCV_sprintf(disasm, sizeof(disasm), "%10s", "sllw");
                break;
            case 4:
                if (((instr >> 25) & 0x7f) == 0x01) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "divw");
                } else {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "error");
                }
                break;
            case 5:
                if (((instr >> 25) & 0x7f) == 0x00) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "srlw");
                } else if (((instr >> 25) & 0x7f) == 0x01) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "divuw");
                } else if (((instr >> 25) & 0x7f) == 0x20) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "sraw");
                } else {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "error");
                }
                break;
            case 6:
                if (((instr >> 25) & 0x7f) == 0x01) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "remw");
                } else {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "error");
                }
                break;
            case 7:
                if (((instr >> 25) & 0x7f) == 0x01) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "remuw");
                } else {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "error");
                }
                break;
            default: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "error");
            }
            break;
        case 0x18:
            switch (op2) {
            case 0: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "beq"); break;
            case 1: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "bne"); break;
            case 4: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "blt"); break;
            case 5: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "bge"); break;
            case 6: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "bltu"); break;
            case 7: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "bgeu"); break;
            default: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "error");
            }
            break;
        case 0x19:
            RISCV_sprintf(disasm, sizeof(disasm), "%10s", "jalr");
            break;
        case 0x1B :
            RISCV_sprintf(disasm, sizeof(disasm), "%10s", "jal");
            break;
        case 0x1C:
            switch (op2) {
            case 0:
                if (instr == 0x00000073) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "ecall");
                } else if (instr == 0x00100073) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "ebreak");
                } else if (instr == 0x00200073) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "uret");
                } else if (instr == 0x10200073) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "sret");
                } else if (instr == 0x20200073) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "hret");
                } else if (instr == 0x30200073) {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "mret");
                } else {
                    RISCV_sprintf(disasm, sizeof(disasm), "%10s", "error");
                }
                break;
            case 1:
                RISCV_sprintf(disasm, sizeof(disasm), "%10s", "csrrw");
                break;
            case 2:
                RISCV_sprintf(disasm, sizeof(disasm), "%10s", "csrrs");
                break;
            case 3:
                RISCV_sprintf(disasm, sizeof(disasm), "%10s", "csrrc");
                break;
            case 5:
                RISCV_sprintf(disasm, sizeof(disasm), "%10s", "csrrwi");
                break;
            case 6:
                RISCV_sprintf(disasm, sizeof(disasm), "%10s", "csrrsi");
                break;
            case 7:
                RISCV_sprintf(disasm, sizeof(disasm), "%10s", "csrrci");
                break;
            default:
                RISCV_sprintf(disasm, sizeof(disasm), "%10s", "error");
            }
            break;
        default:
            RISCV_sprintf(disasm, sizeof(disasm), "%10s", "unknown");
        }
    }
}

void Tracer::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_dbg_executed_cnt, i_dbg_executed_cnt.name());
        sc_trace(o_vcd, i_e_valid, i_e_valid.name());
        sc_trace(o_vcd, i_m_wena, i_m_wena.name());
        sc_trace(o_vcd, i_reg_ignored, i_reg_ignored.name());

        std::string pn(name());
        sc_trace(o_vcd, tr_wcnt_, pn + ".tr_wcnt_");
        sc_trace(o_vcd, tr_rcnt_, pn + ".tr_rcnt_");
        sc_trace(o_vcd, tr_total_, pn + ".tr_total_");
    }
}

void Tracer::registers() {
    TraceStepType *p_e_wr = &trace_tbl_[tr_wcnt_];

    if (i_e_valid.read() == 1) {
        p_e_wr->exec_cnt = i_dbg_executed_cnt.read() + 1;
        p_e_wr->pc = i_e_pc.read();
        p_e_wr->instr = i_e_instr.read().to_uint();

        tr_wcnt_ = (tr_wcnt_ + 1) % TRACE_TBL_SZ;
        // Clear next element:
        memset(&trace_tbl_[tr_wcnt_], 0, sizeof(trace_tbl_[tr_wcnt_]));
    }

    if (i_e_memop_valid.read() == 1 && i_m_memop_ready.read() == 1) {
        MemopActionType *pm = &p_e_wr->memaction[p_e_wr->memactioncnt++];
        pm->store = i_e_memop_type.read()[MemopType_Store];
        pm->memaddr = i_e_memop_addr.read();
        pm->size = i_e_memop_size.read().to_int();
        pm->data = i_e_memop_wdata.read();
        pm->regaddr = i_e_waddr.read();
        if (i_e_waddr.read().or_reduce() == 0 ||
            (i_e_memop_type.read()[MemopType_Store] == 1 && i_e_memop_type.read()[MemopType_Release] == 0)) {
            pm->complete = 1;
        }
        pm->sc_release = i_e_memop_type.read()[MemopType_Release];
    }

    if (i_e_wena.read() == 1) {
        // Direct register writting if it is not a Load operation
        RegActionType *pr = &p_e_wr->regaction[p_e_wr->regactioncnt++];
        pr->waddr = i_e_waddr.read();
        pr->wres = i_e_wdata.read();
    } else if (i_m_wena.read()) {
        // Update current rd memory action (memory operations are strictly ordered)
        for (int i = 0; i < trace_tbl_[tr_rcnt_].memactioncnt; i++) {
            TraceStepType *p = &trace_tbl_[tr_rcnt_];
            MemopActionType *pm = &p->memaction[i];
            if (!pm->complete) {
                pm->complete = 1;
                pm->ignored = i_reg_ignored.read();
                if (pm->sc_release) {
                    if (i_m_wdata.read() == 1) {
                        pm->ignored = 1;
                    }
                    // do not re-write stored value by returning error status
                } else {
                    pm->data = i_m_wdata.read();
                }

                if (!i_reg_ignored.read()) {
                    RegActionType *pr = &p->regaction[p->regactioncnt++];
                    pr->waddr = i_m_waddr.read();
                    pr->wres = i_m_wdata.read();
                }
                break;
            }
        }
    }
   

    // check instruction data completness
    bool entry_valid;
    while (tr_rcnt_ != tr_wcnt_) {
        entry_valid = true;
        for (int i = 0; i < trace_tbl_[tr_rcnt_].memactioncnt; i++) {
            if (!trace_tbl_[tr_rcnt_].memaction[i].complete) {
                entry_valid = false;
                break;
            }
        }
        if (!entry_valid) {
            break;
        }
        trace_output(&trace_tbl_[tr_rcnt_]);
        tr_rcnt_ = (tr_rcnt_ + 1) %  TRACE_TBL_SZ;
    }
}

void Tracer::trace_output(TraceStepType *tr) {
    char msg[256];
    int tsz;
    uint64_t msk[4] = {0xFFull, 0xFFFFull, 0xFFFFFFFFull, ~0ull};

    task_disassembler(tr->instr);
    tsz = RISCV_sprintf(msg, sizeof(msg),
        "%9" RV_PRI64 "d: %08" RV_PRI64 "x: %s \n",
            tr->exec_cnt,
            tr->pc,
            disasm);
    fwrite(msg, 1, tsz, fl_);

    for (int i = 0; i < tr->memactioncnt; i++) {
        MemopActionType *pm = &tr->memaction[i];
        if (pm->ignored) {
            continue;
        }
        if (pm->store == 0) {
            tsz = RISCV_sprintf(msg, sizeof(msg),
                "%20s [%08" RV_PRI64 "x] => %016" RV_PRI64 "x\n",
                    "",
                    pm->memaddr,
                    pm->data & msk[pm->size]);
            fwrite(msg, 1, tsz, fl_);
        } else {
            tsz = RISCV_sprintf(msg, sizeof(msg),
                "%20s [%08" RV_PRI64 "x] <= %016" RV_PRI64 "x\n",
                    "",
                    pm->memaddr,
                    pm->data & msk[pm->size]);
            fwrite(msg, 1, tsz, fl_);
        }
    }

    for (int i = 0; i < tr->regactioncnt; i++) {
        RegActionType *pr = &tr->regaction[i];
            tsz = RISCV_sprintf(msg, sizeof(msg),
                "%20s %10s <= %016" RV_PRI64 "x\n",
                    "",
                    rname[pr->waddr],
                    pr->wres);
            fwrite(msg, 1, tsz, fl_);
    }
}

}  // namespace debugger

