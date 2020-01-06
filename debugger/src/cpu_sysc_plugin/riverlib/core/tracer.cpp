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
    i_e_multi_ready("i_e_multi_ready"),
    i_e_wena("i_e_wena"),
    i_e_whazard("i_e_whazard"),
    i_e_waddr("i_e_waddr"),
    i_e_wdata("i_e_wdata"),
    i_e_memop_store("i_e_memop_store"),
    i_e_memop_load("i_e_memop_load"),
    i_e_memop_addr("i_e_memop_addr"),
    i_e_memop_wdata("i_e_memop_wdata"),
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
    sensitive << i_e_instr;
    sensitive << i_e_multi_ready;
    sensitive << i_e_wena;
    sensitive << i_e_whazard;
    sensitive << i_e_waddr;
    sensitive << i_e_wdata;
    sensitive << i_e_memop_store;
    sensitive << i_e_memop_load;
    sensitive << i_e_memop_addr;
    sensitive << i_e_memop_wdata;
    sensitive << i_m_wena;
    sensitive << i_m_waddr;
    sensitive << i_m_wdata;

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
        sc_trace(o_vcd, i_e_valid, i_e_valid.name());
        sc_trace(o_vcd, i_m_wena, i_m_wena.name());

        std::string pn(name());
        sc_trace(o_vcd, tr_wcnt_, pn + ".tr_wcnt_");
        sc_trace(o_vcd, tr_rcnt_, pn + ".tr_rcnt_");
        sc_trace(o_vcd, tr_total_, pn + ".tr_total_");
    }
}

void Tracer::registers() {
    char msg[256];
    int tsz;


    if (i_e_valid.read() == 1) {
        TraceStepType *p = &trace_tbl_[tr_wcnt_];
        if (i_e_multi_ready.read() == 0) {
            tr_total_++;
            if (++tr_wcnt_ >=  TRACE_TBL_SZ) {
                tr_wcnt_ = 0;
            }
        }

        p->exec_cnt = i_dbg_executed_cnt.read();
        p->pc = i_e_pc.read();
        p->instr = i_e_instr.read().to_uint();
        p->memop_addr = i_e_memop_addr.read();
        p->memop_load = i_e_memop_load.read();
        p->memop_store = i_e_memop_store.read();
        p->entry_valid = !p->whazard;
    }

    // 1 clock delay
    trace_tbl_[tr_wcnt_].waddr = i_e_waddr.read().to_uint();
    trace_tbl_[tr_wcnt_].wres = i_e_wdata.read();
    trace_tbl_[tr_wcnt_].whazard = i_e_whazard.read();
    if (i_e_multi_ready.read() == 1) {
        tr_total_++;
        if (++tr_wcnt_ >=  TRACE_TBL_SZ) {
            tr_wcnt_ = 0;
        }
    }

    if (!i_e_wena.read() && i_m_wena.read() && trace_tbl_[tr_rcnt_].whazard) {
        TraceStepType *p = &trace_tbl_[tr_rcnt_];
        p->entry_valid = 1;
        p->whazard = 0;
        p->wres = i_m_wdata.read();
    }

    while (trace_tbl_[tr_rcnt_].entry_valid) {
        TraceStepType *p = &trace_tbl_[tr_rcnt_];
        p->entry_valid = 0;
        tr_total_--;
        if (++tr_rcnt_ >=  TRACE_TBL_SZ) {
            tr_rcnt_ = 0;
        }

        task_disassembler(p->instr);
        tsz = RISCV_sprintf(msg, sizeof(msg),
            "%9" RV_PRI64 "d: %08" RV_PRI64 "x: %s \n",
                p->exec_cnt,
                p->pc,
                disasm);
        fwrite(msg, 1, tsz, fl_);

        if (p->memop_load) {
            tsz = RISCV_sprintf(msg, sizeof(msg),
                "%20s [%08" RV_PRI64 "x] => %016" RV_PRI64 "x\n",
                    "",
                    p->memop_addr,
                    p->wres);
            fwrite(msg, 1, tsz, fl_);
            tsz = RISCV_sprintf(msg, sizeof(msg),
                "%20s %10s <= %016" RV_PRI64 "x\n",
                    "",
                    rname[p->waddr],
                    p->wres);
            fwrite(msg, 1, tsz, fl_);
        } else {
            if (p->waddr != 0) {
                tsz = RISCV_sprintf(msg, sizeof(msg),
                    "%20s %10s <= %016" RV_PRI64 "x\n",
                        "",
                        rname[p->waddr],
                        p->wres);
                fwrite(msg, 1, tsz, fl_);
            }

            if (p->memop_store) {
                tsz = RISCV_sprintf(msg, sizeof(msg),
                    "%20s [%08" RV_PRI64 "x] <= %016" RV_PRI64 "x\n",
                        "",
                        p->memop_addr,
                        p->wres);
                fwrite(msg, 1, tsz, fl_);
            }
        }

    }
}

}  // namespace debugger

