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
    i_e_memop_store("i_e_memop_store"),
    i_e_memop_load("i_e_memop_load"),
    i_e_memop_addr("i_e_memop_addr"),
    i_e_res_data("i_e_res_data"),
    i_e_res_addr("i_e_res_addr"),
    i_m_wena("i_m_wena"),
    i_m_waddr("i_m_waddr"),
    i_m_wdata("i_m_wdata") {
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
    sensitive << i_e_memop_store;
    sensitive << i_e_memop_load;
    sensitive << i_e_memop_addr;
    sensitive << i_e_res_data;
    sensitive << i_e_res_addr;
    sensitive << i_m_wena;
    sensitive << i_m_waddr;
    sensitive << i_m_wdata;

    SC_METHOD(registers);
    sensitive << i_clk.pos();

    r_load_reg = 0;
};

void Tracer::comb() {

}

void Tracer::task_disassembler(uint32_t instr) {
    if ((instr & 0x3) != 3) {
        RISCV_sprintf(disasm, sizeof(disasm), "%10s", "c_unknown");
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
        case 4:
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
        case 0x08:
            switch (op2) {
            case 0: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "sb"); break;
            case 1: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "sh"); break;
            case 2: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "sw"); break;
            case 3: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "sd"); break;
            default: RISCV_sprintf(disasm, sizeof(disasm), "%10s", "error");
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
                RISCV_sprintf(disasm, sizeof(disasm), "%10s", "sll");
                break;
            case 2:
                RISCV_sprintf(disasm, sizeof(disasm), "%10s", "slt");
                break;
            case 3:
                RISCV_sprintf(disasm, sizeof(disasm), "%10s", "sltu");
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
        case 0x1B :
            RISCV_sprintf(disasm, sizeof(disasm), "%10s", "jal");
            break;
        case 0x19:
            RISCV_sprintf(disasm, sizeof(disasm), "%10s", "jalr");
            break;
        default:
            RISCV_sprintf(disasm, sizeof(disasm), "%10s", "unknown");
        }
    }
}

void Tracer::registers() {
    char msg[256];
    int tsz;
    if (r_load_reg.read() == 1 && i_m_wena.read() == 1) {
        tsz = RISCV_sprintf(msg, sizeof(msg),
            "%20s [%08" RV_PRI64 "x] => %016" RV_PRI64 "x\n",
                "",
                r_load_addr.read().to_uint64(),
                i_m_wdata.read().to_uint64());
        fwrite(msg, 1, tsz, fl_);
        tsz = RISCV_sprintf(msg, sizeof(msg),
            "%20s %10s <= %016" RV_PRI64 "x\n",
                "",
                rname[i_m_waddr.read().to_uint()],
                i_m_wdata.read().to_uint64());
        fwrite(msg, 1, tsz, fl_);
    }
    r_load_reg = 0;

    if (i_e_valid.read() == 1) {
        task_disassembler(i_e_instr.read().to_uint());
        tsz = RISCV_sprintf(msg, sizeof(msg),
            "%9" RV_PRI64 "d: %08" RV_PRI64 "x: %s \n",
                i_dbg_executed_cnt.read().to_uint64(),
                i_e_pc.read().to_uint64(),
                disasm);
        fwrite(msg, 1, tsz, fl_);

        if (i_e_memop_load.read() == 0) {
            if (i_e_res_addr.read() != 0) {
                tsz = RISCV_sprintf(msg, sizeof(msg),
                    "%20s %10s <= %016" RV_PRI64 "x\n",
                        "",
                        rname[i_e_res_addr.read().to_uint()],
                        i_e_res_data.read().to_uint64());
                fwrite(msg, 1, tsz, fl_);
            }

            if (i_e_memop_store.read() == 1) {
                tsz = RISCV_sprintf(msg, sizeof(msg),
                    "%20s [%08" RV_PRI64 "x] <= %016" RV_PRI64 "x\n",
                        "",
                        i_e_memop_addr.read().to_uint64(),
                        i_e_res_data.read().to_uint64());
                fwrite(msg, 1, tsz, fl_);
            }
        } else {
            r_load_reg = 1;
            r_load_addr = i_e_memop_addr.read();
        }
    }
}

}  // namespace debugger

