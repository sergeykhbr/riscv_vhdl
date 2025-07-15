// 
//  Copyright 2022 Sergey Khabarov, sergeykhbr@gmail.com
// 
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
// 
//      http://www.apache.org/licenses/LICENSE-2.0
// 
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
// 

#include "tracer.h"
#include "api_core.h"

namespace debugger {

static const std::string rname[64] = {
    "zero",  // x0
    "ra",  // x1
    "sp",  // x2
    "gp",  // x3
    "tp",  // x4
    "t0",  // x5
    "t1",  // x6
    "t2",  // x7
    "s0",  // x8
    "s1",  // x9
    "a0",  // x10
    "a1",  // x11
    "a2",  // x12
    "a3",  // x13
    "a4",  // x14
    "a5",  // x15
    "a6",  // x16
    "a7",  // x17
    "s2",  // x18
    "s3",  // x19
    "s4",  // x20
    "s5",  // x21
    "s6",  // x22
    "s7",  // x23
    "s8",  // x24
    "s9",  // x25
    "s10",  // x26
    "s11",  // x27
    "t3",  // x28
    "t4",  // x29
    "t5",  // x30
    "t6",  // x31
    "ft0",  // x32
    "ft1",  // x33
    "ft2",  // x34
    "ft3",  // x35
    "ft4",  // x36
    "ft5",  // x37
    "ft6",  // x38
    "ft7",  // x39
    "fs0",  // x40
    "fs1",  // x41
    "fa0",  // x42
    "fa1",  // x43
    "fa2",  // x44
    "fa3",  // x45
    "fa4",  // x46
    "fa5",  // x47
    "fa6",  // x48
    "fa7",  // x49
    "fs2",  // x50
    "fs3",  // x51
    "fs4",  // x52
    "fs5",  // x53
    "fs6",  // x54
    "fs7",  // x55
    "fs8",  // x56
    "fs9",  // x57
    "fs10",  // x58
    "fs11",  // x59
    "ft8",  // x60
    "ft9",  // x61
    "ft10",  // x62
    "ft11"  // x63
};

Tracer::Tracer(sc_module_name name,
               bool async_reset,
               uint32_t hartid,
               std::string trace_file)
    : sc_module(name),
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
    i_e_flushd("i_e_flushd"),
    i_m_pc("i_m_pc"),
    i_m_valid("i_m_valid"),
    i_m_memop_ready("i_m_memop_ready"),
    i_m_wena("i_m_wena"),
    i_m_waddr("i_m_waddr"),
    i_m_wdata("i_m_wdata"),
    i_reg_ignored("i_reg_ignored") {

    async_reset_ = async_reset;
    hartid_ = hartid;
    trace_file_ = trace_file;

    SC_THREAD(init);

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
    sensitive << i_e_flushd;
    sensitive << i_m_pc;
    sensitive << i_m_valid;
    sensitive << i_m_memop_ready;
    sensitive << i_m_wena;
    sensitive << i_m_waddr;
    sensitive << i_m_wdata;
    sensitive << i_reg_ignored;
    for (int i = 0; i < TRACE_TBL_SZ; i++) {
        sensitive << r.trace_tbl[i].exec_cnt;
        sensitive << r.trace_tbl[i].pc;
        sensitive << r.trace_tbl[i].instr;
        sensitive << r.trace_tbl[i].regactioncnt;
        sensitive << r.trace_tbl[i].memactioncnt;
        for (int j = 0; j < TRACE_TBL_SZ; j++) {
            sensitive << r.trace_tbl[i].regaction[j].waddr;
            sensitive << r.trace_tbl[i].regaction[j].wres;
        }
        for (int j = 0; j < TRACE_TBL_SZ; j++) {
            sensitive << r.trace_tbl[i].memaction[j].store;
            sensitive << r.trace_tbl[i].memaction[j].size;
            sensitive << r.trace_tbl[i].memaction[j].mask;
            sensitive << r.trace_tbl[i].memaction[j].memaddr;
            sensitive << r.trace_tbl[i].memaction[j].data;
            sensitive << r.trace_tbl[i].memaction[j].regaddr;
            sensitive << r.trace_tbl[i].memaction[j].complete;
            sensitive << r.trace_tbl[i].memaction[j].sc_release;
            sensitive << r.trace_tbl[i].memaction[j].ignored;
        }
        sensitive << r.trace_tbl[i].completed;
    }
    sensitive << r.tr_wcnt;
    sensitive << r.tr_rcnt;
    sensitive << r.tr_total;
    sensitive << r.tr_opened;

    SC_METHOD(traceout);
    sensitive << i_clk.pos();

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void Tracer::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_dbg_executed_cnt, i_dbg_executed_cnt.name());
        sc_trace(o_vcd, i_e_valid, i_e_valid.name());
        sc_trace(o_vcd, i_e_pc, i_e_pc.name());
        sc_trace(o_vcd, i_e_instr, i_e_instr.name());
        sc_trace(o_vcd, i_e_wena, i_e_wena.name());
        sc_trace(o_vcd, i_e_waddr, i_e_waddr.name());
        sc_trace(o_vcd, i_e_wdata, i_e_wdata.name());
        sc_trace(o_vcd, i_e_memop_valid, i_e_memop_valid.name());
        sc_trace(o_vcd, i_e_memop_type, i_e_memop_type.name());
        sc_trace(o_vcd, i_e_memop_size, i_e_memop_size.name());
        sc_trace(o_vcd, i_e_memop_addr, i_e_memop_addr.name());
        sc_trace(o_vcd, i_e_memop_wdata, i_e_memop_wdata.name());
        sc_trace(o_vcd, i_e_flushd, i_e_flushd.name());
        sc_trace(o_vcd, i_m_pc, i_m_pc.name());
        sc_trace(o_vcd, i_m_valid, i_m_valid.name());
        sc_trace(o_vcd, i_m_memop_ready, i_m_memop_ready.name());
        sc_trace(o_vcd, i_m_wena, i_m_wena.name());
        sc_trace(o_vcd, i_m_waddr, i_m_waddr.name());
        sc_trace(o_vcd, i_m_wdata, i_m_wdata.name());
        sc_trace(o_vcd, i_reg_ignored, i_reg_ignored.name());
        for (int i = 0; i < TRACE_TBL_SZ; i++) {
            sc_trace(o_vcd, r.trace_tbl[i].exec_cnt, pn + ".r.trace_tbl(" + std::to_string(i) + ").exec_cnt");
            sc_trace(o_vcd, r.trace_tbl[i].pc, pn + ".r.trace_tbl(" + std::to_string(i) + ").pc");
            sc_trace(o_vcd, r.trace_tbl[i].instr, pn + ".r.trace_tbl(" + std::to_string(i) + ").instr");
            sc_trace(o_vcd, r.trace_tbl[i].regactioncnt, pn + ".r.trace_tbl(" + std::to_string(i) + ").regactioncnt");
            sc_trace(o_vcd, r.trace_tbl[i].memactioncnt, pn + ".r.trace_tbl(" + std::to_string(i) + ").memactioncnt");
            for (int j = 0; j < TRACE_TBL_SZ; j++) {
                sc_trace(o_vcd, r.trace_tbl[i].regaction[j].waddr, pn + ".r.trace_tbl(" + std::to_string(i) + ").regaction(" + std::to_string(j) + ").waddr");
                sc_trace(o_vcd, r.trace_tbl[i].regaction[j].wres, pn + ".r.trace_tbl(" + std::to_string(i) + ").regaction(" + std::to_string(j) + ").wres");
            }
            for (int j = 0; j < TRACE_TBL_SZ; j++) {
                sc_trace(o_vcd, r.trace_tbl[i].memaction[j].store, pn + ".r.trace_tbl(" + std::to_string(i) + ").memaction(" + std::to_string(j) + ").store");
                sc_trace(o_vcd, r.trace_tbl[i].memaction[j].size, pn + ".r.trace_tbl(" + std::to_string(i) + ").memaction(" + std::to_string(j) + ").size");
                sc_trace(o_vcd, r.trace_tbl[i].memaction[j].mask, pn + ".r.trace_tbl(" + std::to_string(i) + ").memaction(" + std::to_string(j) + ").mask");
                sc_trace(o_vcd, r.trace_tbl[i].memaction[j].memaddr, pn + ".r.trace_tbl(" + std::to_string(i) + ").memaction(" + std::to_string(j) + ").memaddr");
                sc_trace(o_vcd, r.trace_tbl[i].memaction[j].data, pn + ".r.trace_tbl(" + std::to_string(i) + ").memaction(" + std::to_string(j) + ").data");
                sc_trace(o_vcd, r.trace_tbl[i].memaction[j].regaddr, pn + ".r.trace_tbl(" + std::to_string(i) + ").memaction(" + std::to_string(j) + ").regaddr");
                sc_trace(o_vcd, r.trace_tbl[i].memaction[j].complete, pn + ".r.trace_tbl(" + std::to_string(i) + ").memaction(" + std::to_string(j) + ").complete");
                sc_trace(o_vcd, r.trace_tbl[i].memaction[j].sc_release, pn + ".r.trace_tbl(" + std::to_string(i) + ").memaction(" + std::to_string(j) + ").sc_release");
                sc_trace(o_vcd, r.trace_tbl[i].memaction[j].ignored, pn + ".r.trace_tbl(" + std::to_string(i) + ").memaction(" + std::to_string(j) + ").ignored");
            }
            sc_trace(o_vcd, r.trace_tbl[i].completed, pn + ".r.trace_tbl(" + std::to_string(i) + ").completed");
        }
        sc_trace(o_vcd, r.tr_wcnt, pn + ".r.tr_wcnt");
        sc_trace(o_vcd, r.tr_rcnt, pn + ".r.tr_rcnt");
        sc_trace(o_vcd, r.tr_total, pn + ".r.tr_total");
        sc_trace(o_vcd, r.tr_opened, pn + ".r.tr_opened");
    }

}

std::string Tracer::TaskDisassembler(sc_uint<32> instr) {
    char tstr[256];
    std::string ostr;

    if (instr(1, 0) != 3) {
        switch (instr(1, 0)) {
        case 0:
            switch (instr(15, 13)) {
            case 0:
                if (instr(12, 2).or_reduce() == 0) {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                } else {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.addi4spn");
                    ostr = std::string(tstr);
                }
                break;
            case 1:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.fld");
                ostr = std::string(tstr);
                break;
            case 2:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.lw");
                ostr = std::string(tstr);
                break;
            case 3:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.ld");
                ostr = std::string(tstr);
                break;
            case 4:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                ostr = std::string(tstr);
                break;
            case 5:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.fsd");
                ostr = std::string(tstr);
                break;
            case 6:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.sw");
                ostr = std::string(tstr);
                break;
            case 7:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.sd");
                ostr = std::string(tstr);
                break;
            }
            break;
        case 1:
            switch (instr(15, 13)) {
            case 0:
                if (instr(12, 2).or_reduce() == 0) {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.nop");
                    ostr = std::string(tstr);
                } else {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.addi");
                    ostr = std::string(tstr);
                }
                break;
            case 1:
                if (instr(11, 7).or_reduce() == 0) {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                } else {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.addiw");
                    ostr = std::string(tstr);
                }
                break;
            case 2:
                if (instr(11, 7).or_reduce() == 0) {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                } else {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.li");
                    ostr = std::string(tstr);
                }
                break;
            case 3:
                if (instr(11, 7) == 2) {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.addi16sp");
                    ostr = std::string(tstr);
                } else if (instr(11, 7).or_reduce() == 1) {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.lui");
                    ostr = std::string(tstr);
                } else {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                }
                break;
            case 4:
                if (instr(11, 10) == 0) {
                    if ((instr[12] == 0) && (instr(6, 2) == 0)) {
                        RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.srli64");
                        ostr = std::string(tstr);
                    } else {
                        RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.srli");
                        ostr = std::string(tstr);
                    }
                } else if (instr(11, 10) == 1) {
                    if ((instr[12] == 0) && (instr(6, 2) == 0)) {
                        RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.srai64");
                        ostr = std::string(tstr);
                    } else {
                        RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.srai");
                        ostr = std::string(tstr);
                    }
                } else if (instr(11, 10) == 2) {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.andi");
                    ostr = std::string(tstr);
                } else {
                    if ((instr[12] == 0) && (instr(6, 5) == 0)) {
                        RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.sub");
                        ostr = std::string(tstr);
                    } else if ((instr[12] == 0) && (instr(6, 5) == 1)) {
                        RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.xor");
                        ostr = std::string(tstr);
                    } else if ((instr[12] == 0) && (instr(6, 5) == 2)) {
                        RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.or");
                        ostr = std::string(tstr);
                    } else if ((instr[12] == 0) && (instr(6, 5) == 3)) {
                        RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.and");
                        ostr = std::string(tstr);
                    } else if ((instr[12] == 1) && (instr(6, 5) == 0)) {
                        RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.subw");
                        ostr = std::string(tstr);
                    } else if ((instr[12] == 1) && (instr(6, 5) == 1)) {
                        RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.addw");
                        ostr = std::string(tstr);
                    } else {
                        RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                        ostr = std::string(tstr);
                    }
                }
                break;
            case 5:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.j");
                ostr = std::string(tstr);
                break;
            case 6:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.beqz");
                ostr = std::string(tstr);
                break;
            case 7:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.bnez");
                ostr = std::string(tstr);
                break;
            }
            break;
        case 2:
            switch (instr(15, 13)) {
            case 0:
                if ((instr[12] == 0) && (instr(6, 5) == 0)) {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.slli64");
                    ostr = std::string(tstr);
                } else {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.slli");
                    ostr = std::string(tstr);
                }
                break;
            case 1:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.fldsp");
                ostr = std::string(tstr);
                break;
            case 2:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.lwsp");
                ostr = std::string(tstr);
                break;
            case 3:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.ldsp");
                ostr = std::string(tstr);
                break;
            case 4:
                if ((instr[12] == 0) && (instr(6, 2) == 0)) {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.jr");
                    ostr = std::string(tstr);
                } else if ((instr[12] == 0) && (instr(6, 2) != 0)) {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.mv");
                    ostr = std::string(tstr);
                } else if ((instr[12] == 1) && (instr(6, 2) == 0) && (instr(11, 7) == 0)) {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.ebreak");
                    ostr = std::string(tstr);
                } else if ((instr[12] == 1) && (instr(6, 2) == 0)) {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.jalr");
                    ostr = std::string(tstr);
                } else {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.add");
                    ostr = std::string(tstr);
                }
                break;
            case 5:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.fsdsp");
                ostr = std::string(tstr);
                break;
            case 6:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.swsp");
                ostr = std::string(tstr);
                break;
            case 7:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "c.sdsp");
                ostr = std::string(tstr);
                break;
            }
            break;
        default:
            RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
            ostr = std::string(tstr);
            break;
        }
    } else {
        // RV decoder
        switch (instr(6, 0)) {
        case 0x03:
            switch (instr(14, 12)) {
            case 0:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "lb");
                ostr = std::string(tstr);
                break;
            case 1:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "lh");
                ostr = std::string(tstr);
                break;
            case 2:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "lw");
                ostr = std::string(tstr);
                break;
            case 3:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ld");
                ostr = std::string(tstr);
                break;
            case 4:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "lbu");
                ostr = std::string(tstr);
                break;
            case 5:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "lhu");
                ostr = std::string(tstr);
                break;
            case 6:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "lwu");
                ostr = std::string(tstr);
                break;
            default:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                ostr = std::string(tstr);
                break;
            }
            break;
        case 0x07:
            switch (instr(14, 12)) {
            case 3:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "fld");
                ostr = std::string(tstr);
                break;
            default:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                ostr = std::string(tstr);
                break;
            }
            break;
        case 0x0F:
            switch (instr(14, 12)) {
            case 0:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "fence");
                ostr = std::string(tstr);
                break;
            case 1:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "fence.i");
                ostr = std::string(tstr);
                break;
            default:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                ostr = std::string(tstr);
                break;
            }
            break;
        case 0x13:
            switch (instr(14, 12)) {
            case 0:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "addi");
                ostr = std::string(tstr);
                break;
            case 1:
                if (instr(31, 26).or_reduce() == 0) {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "slli");
                    ostr = std::string(tstr);
                } else {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                }
                break;
            case 2:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "slti");
                ostr = std::string(tstr);
                break;
            case 3:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "sltiu");
                ostr = std::string(tstr);
                break;
            case 4:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "xori");
                ostr = std::string(tstr);
                break;
            case 5:
                if (instr(31, 26).or_reduce() == 0) {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "srli");
                    ostr = std::string(tstr);
                } else if (instr(31, 26) == 0x10) {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "srai");
                    ostr = std::string(tstr);
                } else {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                }
                break;
            case 6:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ori");
                ostr = std::string(tstr);
                break;
            case 7:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "andi");
                ostr = std::string(tstr);
                break;
            default:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                ostr = std::string(tstr);
                break;
            }
            break;
        case 0x17:
            RISCV_sprintf(tstr, sizeof(tstr), "%10s", "auipc");
            ostr = std::string(tstr);
            break;
        case 0x1B:
            switch (instr(14, 12)) {
            case 0:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "addiw");
                ostr = std::string(tstr);
                break;
            case 1:
                if (instr(31, 25) == 0x00) {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "slliw");
                    ostr = std::string(tstr);
                } else {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                }
                break;
            case 5:
                switch (instr(31, 25)) {
                case 0:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "srliw");
                    ostr = std::string(tstr);
                    break;
                case 0x20:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "sraiw");
                    ostr = std::string(tstr);
                    break;
                default:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                    break;
                }
                break;
            default:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                ostr = std::string(tstr);
                break;
            }
            break;
        case 0x23:
            switch (instr(14, 12)) {
            case 0:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "sb");
                ostr = std::string(tstr);
                break;
            case 1:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "sh");
                ostr = std::string(tstr);
                break;
            case 2:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "sw");
                ostr = std::string(tstr);
                break;
            case 3:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "sd");
                ostr = std::string(tstr);
                break;
            default:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                ostr = std::string(tstr);
                break;
            }
            break;
        case 0x27:
            switch (instr(14, 12)) {
            case 3:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "fsd");
                ostr = std::string(tstr);
                break;
            default:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                ostr = std::string(tstr);
                break;
            }
            break;
        case 0x2F:
            if (instr(14, 12) == 2) {
                switch (instr(31, 27)) {
                case 0x00:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "amoadd.w");
                    ostr = std::string(tstr);
                    break;
                case 0x01:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "amoswap.w");
                    ostr = std::string(tstr);
                    break;
                case 0x02:
                    if (instr(24, 20).or_reduce() == 0) {
                        RISCV_sprintf(tstr, sizeof(tstr), "%10s", "lr.w");
                        ostr = std::string(tstr);
                    } else {
                        RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                        ostr = std::string(tstr);
                    }
                    break;
                case 0x03:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "sc.w");
                    ostr = std::string(tstr);
                    break;
                case 0x04:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "amoxor.w");
                    ostr = std::string(tstr);
                    break;
                case 0x08:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "amoor.w");
                    ostr = std::string(tstr);
                    break;
                case 0x0C:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "amoand.w");
                    ostr = std::string(tstr);
                    break;
                case 0x10:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "amomin.w");
                    ostr = std::string(tstr);
                    break;
                case 0x14:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "amomax.w");
                    ostr = std::string(tstr);
                    break;
                case 0x18:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "amominu.w");
                    ostr = std::string(tstr);
                    break;
                case 0x1C:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "amomaxu.w");
                    ostr = std::string(tstr);
                    break;
                default:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                    break;
                }
            } else if (instr(14, 12) == 3) {
                switch (instr(31, 27)) {
                case 0x00:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "amoadd.d");
                    ostr = std::string(tstr);
                    break;
                case 0x01:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "amoswap.d");
                    ostr = std::string(tstr);
                    break;
                case 0x02:
                    if (instr(24, 20).or_reduce() == 0) {
                        RISCV_sprintf(tstr, sizeof(tstr), "%10s", "lr.d");
                        ostr = std::string(tstr);
                    } else {
                        RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                        ostr = std::string(tstr);
                    }
                    break;
                case 0x03:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "sc.d");
                    ostr = std::string(tstr);
                    break;
                case 0x04:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "amoxor.d");
                    ostr = std::string(tstr);
                    break;
                case 0x08:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "amoor.d");
                    ostr = std::string(tstr);
                    break;
                case 0x0C:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "amoand.d");
                    ostr = std::string(tstr);
                    break;
                case 0x10:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "amomin.d");
                    ostr = std::string(tstr);
                    break;
                case 0x14:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "amomax.d");
                    ostr = std::string(tstr);
                    break;
                case 0x18:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "amominu.d");
                    ostr = std::string(tstr);
                    break;
                case 0x1C:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "amomaxu.d");
                    ostr = std::string(tstr);
                    break;
                default:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                    break;
                }
            } else {
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                ostr = std::string(tstr);
            }
            break;
        case 0x33:
            switch (instr(14, 12)) {
            case 0:
                switch (instr(31, 25)) {
                case 0:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "add");
                    ostr = std::string(tstr);
                    break;
                case 0x01:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "mul");
                    ostr = std::string(tstr);
                    break;
                case 0x20:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "sub");
                    ostr = std::string(tstr);
                    break;
                default:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                    break;
                }
                break;
            case 1:
                switch (instr(31, 25)) {
                case 0:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "sll");
                    ostr = std::string(tstr);
                    break;
                case 0x01:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "mulh");
                    ostr = std::string(tstr);
                    break;
                default:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                    break;
                }
                break;
            case 2:
                switch (instr(31, 25)) {
                case 0:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "slt");
                    ostr = std::string(tstr);
                    break;
                case 0x01:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "mulhsu");
                    ostr = std::string(tstr);
                    break;
                default:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                    break;
                }
                break;
            case 3:
                switch (instr(31, 25)) {
                case 0:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "sltu");
                    ostr = std::string(tstr);
                    break;
                case 0x01:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "mulhu");
                    ostr = std::string(tstr);
                    break;
                default:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                    break;
                }
                break;
            case 4:
                switch (instr(31, 25)) {
                case 0:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "xor");
                    ostr = std::string(tstr);
                    break;
                case 0x01:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "div");
                    ostr = std::string(tstr);
                    break;
                default:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                    break;
                }
                break;
            case 5:
                switch (instr(31, 25)) {
                case 0:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "srl");
                    ostr = std::string(tstr);
                    break;
                case 0x01:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "divu");
                    ostr = std::string(tstr);
                    break;
                case 0x20:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "sra");
                    ostr = std::string(tstr);
                    break;
                default:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                    break;
                }
                break;
            case 6:
                switch (instr(31, 25)) {
                case 0:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "or");
                    ostr = std::string(tstr);
                    break;
                case 0x01:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "rem");
                    ostr = std::string(tstr);
                    break;
                default:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                    break;
                }
                break;
            case 7:
                switch (instr(31, 25)) {
                case 0:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "and");
                    ostr = std::string(tstr);
                    break;
                case 0x01:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "remu");
                    ostr = std::string(tstr);
                    break;
                default:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                    break;
                }
                break;
            default:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                ostr = std::string(tstr);
                break;
            }
            break;
        case 0x37:
            RISCV_sprintf(tstr, sizeof(tstr), "%10s", "lui");
            ostr = std::string(tstr);
            break;
        case 0x3B:
            switch (instr(14, 12)) {
            case 0:
                switch (instr(31, 25)) {
                case 0:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "addw");
                    ostr = std::string(tstr);
                    break;
                case 0x01:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "mulw");
                    ostr = std::string(tstr);
                    break;
                case 0x20:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "subw");
                    ostr = std::string(tstr);
                    break;
                default:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                    break;
                }
                break;
            case 1:
                if (instr(31, 25).or_reduce() == 0) {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "sllw");
                    ostr = std::string(tstr);
                } else {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                }
                break;
            case 4:
                switch (instr(31, 25)) {
                case 0x01:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "divw");
                    ostr = std::string(tstr);
                    break;
                default:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                    break;
                }
                break;
            case 5:
                switch (instr(31, 25)) {
                case 0:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "srlw");
                    ostr = std::string(tstr);
                    break;
                case 0x01:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "divuw");
                    ostr = std::string(tstr);
                    break;
                case 0x20:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "sraw");
                    ostr = std::string(tstr);
                    break;
                default:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                    break;
                }
                break;
            case 6:
                switch (instr(31, 25)) {
                case 0x01:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "remw");
                    ostr = std::string(tstr);
                    break;
                default:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                    break;
                }
                break;
            case 7:
                switch (instr(31, 25)) {
                case 0x01:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "remuw");
                    ostr = std::string(tstr);
                    break;
                default:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                    break;
                }
                break;
            default:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                ostr = std::string(tstr);
                break;
            }
            break;
        case 0x53:
            switch (instr(31, 25)) {
            case 0x01:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "fadd");
                ostr = std::string(tstr);
                break;
            case 0x05:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "fsub");
                ostr = std::string(tstr);
                break;
            case 0x09:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "fmul");
                ostr = std::string(tstr);
                break;
            case 0x0D:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "fdiv");
                ostr = std::string(tstr);
                break;
            case 0x15:
                switch (instr(14, 12)) {
                case 0:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "fmin");
                    ostr = std::string(tstr);
                    break;
                case 1:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "fmax");
                    ostr = std::string(tstr);
                    break;
                default:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                    break;
                }
                break;
            case 0x51:
                switch (instr(14, 12)) {
                case 0:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "fle");
                    ostr = std::string(tstr);
                    break;
                case 1:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "flt");
                    ostr = std::string(tstr);
                    break;
                case 2:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "feq");
                    ostr = std::string(tstr);
                    break;
                default:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                    break;
                }
                break;
            case 0x61:
                switch (instr(24, 20)) {
                case 0x00:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "fcvt.w.d");
                    ostr = std::string(tstr);
                    break;
                case 0x01:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "fcvt.wu.d");
                    ostr = std::string(tstr);
                    break;
                case 0x02:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "fcvt.l.d");
                    ostr = std::string(tstr);
                    break;
                case 0x03:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "fcvt.lu.d");
                    ostr = std::string(tstr);
                    break;
                default:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                    break;
                }
                break;
            case 0x69:
                switch (instr(24, 20)) {
                case 0x00:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "fcvt.d.w");
                    ostr = std::string(tstr);
                    break;
                case 0x01:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "fcvt.d.wu");
                    ostr = std::string(tstr);
                    break;
                case 0x02:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "fcvt.d.l");
                    ostr = std::string(tstr);
                    break;
                case 0x03:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "fcvt.d.lu");
                    ostr = std::string(tstr);
                    break;
                default:
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                    break;
                }
                break;
            case 0x71:
                if ((instr(24, 20).or_reduce() == 0) && (instr(14, 12).or_reduce() == 0)) {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "fmov.x.d");
                    ostr = std::string(tstr);
                } else {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                }
                break;
            case 0x79:
                if ((instr(24, 20).or_reduce() == 0) && (instr(14, 12).or_reduce() == 0)) {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "fmov.d.x");
                    ostr = std::string(tstr);
                } else {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                }
                break;
            default:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                ostr = std::string(tstr);
                break;
            }
            break;
        case 0x63:
            switch (instr(14, 12)) {
            case 0:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "beq");
                ostr = std::string(tstr);
                break;
            case 1:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "bne");
                ostr = std::string(tstr);
                break;
            case 4:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "blt");
                ostr = std::string(tstr);
                break;
            case 5:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "bge");
                ostr = std::string(tstr);
                break;
            case 6:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "bltu");
                ostr = std::string(tstr);
                break;
            case 7:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "bgeu");
                ostr = std::string(tstr);
                break;
            default:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                ostr = std::string(tstr);
                break;
            }
            break;
        case 0x67:
            RISCV_sprintf(tstr, sizeof(tstr), "%10s", "jalr");
            ostr = std::string(tstr);
            break;
        case 0x6F:
            RISCV_sprintf(tstr, sizeof(tstr), "%10s", "jal");
            ostr = std::string(tstr);
            break;
        case 0x73:
            switch (instr(14, 12)) {
            case 0:
                if (instr == 0x00000073) {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ecall");
                    ostr = std::string(tstr);
                } else if (instr == 0x00100073) {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ebreak");
                    ostr = std::string(tstr);
                } else if (instr == 0x00200073) {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "uret");
                    ostr = std::string(tstr);
                } else if (instr == 0x10200073) {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "sret");
                    ostr = std::string(tstr);
                } else if (instr == 0x10500073) {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "wfi");
                    ostr = std::string(tstr);
                } else if (instr == 0x20200073) {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "hret");
                    ostr = std::string(tstr);
                } else if (instr == 0x30200073) {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "mret");
                    ostr = std::string(tstr);
                } else {
                    RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                    ostr = std::string(tstr);
                }
                break;
            case 1:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "csrrw");
                ostr = std::string(tstr);
                break;
            case 2:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "csrrs");
                ostr = std::string(tstr);
                break;
            case 3:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "csrrc");
                ostr = std::string(tstr);
                break;
            case 5:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "csrrwi");
                ostr = std::string(tstr);
                break;
            case 6:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "csrrsi");
                ostr = std::string(tstr);
                break;
            case 7:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "csrrci");
                ostr = std::string(tstr);
                break;
            default:
                RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
                ostr = std::string(tstr);
                break;
            }
            break;
        default:
            RISCV_sprintf(tstr, sizeof(tstr), "%10s", "ERROR");
            ostr = std::string(tstr);
            break;
        }
    }
    return ostr;
}

std::string Tracer::TraceOutput(sc_uint<TRACE_TBL_ABITS> rcnt) {
    char tstr[256];
    std::string ostr;
    std::string disasm;
    int ircnt;
    int iwaddr;

    ircnt = rcnt.to_int();

    disasm = TaskDisassembler(r.trace_tbl[ircnt].instr.read());
    RISCV_sprintf(tstr, sizeof(tstr), "%9" RV_PRI64 "d: %08" RV_PRI64 "x: ",
            r.trace_tbl[ircnt].exec_cnt.read().to_uint64(),
            r.trace_tbl[ircnt].pc.read().to_uint64());
    ostr += std::string(tstr);

    ostr += disasm;
    ostr += " \n";

    for (int i = 0; i < r.trace_tbl[ircnt].memactioncnt.read().to_int(); i++) {
        if (r.trace_tbl[ircnt].memaction[i].ignored.read() == 0) {
            if (r.trace_tbl[ircnt].memaction[i].store.read() == 0) {
                RISCV_sprintf(tstr, sizeof(tstr), "%20s [%08" RV_PRI64 "x] => %016" RV_PRI64 "x\n",
                        "",
                        r.trace_tbl[ircnt].memaction[i].memaddr.read().to_uint64(),
                        r.trace_tbl[ircnt].memaction[i].data.read().to_uint64());
                ostr += std::string(tstr);
            } else {
                RISCV_sprintf(tstr, sizeof(tstr), "%20s [%08" RV_PRI64 "x] <= %016" RV_PRI64 "x\n",
                        "",
                        r.trace_tbl[ircnt].memaction[i].memaddr.read().to_uint64(),
                        r.trace_tbl[ircnt].memaction[i].data.read().to_uint64());
                ostr += std::string(tstr);
            }
        }
    }

    for (int i = 0; i < r.trace_tbl[ircnt].regactioncnt.read().to_int(); i++) {
        iwaddr = r.trace_tbl[ircnt].regaction[i].waddr.read().to_int();
        RISCV_sprintf(tstr, sizeof(tstr), "%20s %10s <= %016" RV_PRI64 "x\n",
                "",
                rname[iwaddr].c_str(),
                r.trace_tbl[ircnt].regaction[i].wres.read().to_uint64());
        ostr += std::string(tstr);
    }
    return ostr;
}

void Tracer::init() {
    char tstr[256];
    RISCV_sprintf(tstr, sizeof(tstr), "%s%d.log",
            trace_file_.c_str(),
            hartid_);
    trfilename = std::string(tstr);
    fl = fopen(trfilename.c_str(), "wb");
}

void Tracer::comb() {
    int wcnt;
    int xcnt;
    int rcnt;
    int regcnt;
    int memcnt;
    sc_uint<7> mskoff;
    sc_uint<64> mask;
    sc_uint<TRACE_TBL_ABITS> tr_wcnt_nxt;
    bool checked;
    bool entry_valid;
    sc_uint<TRACE_TBL_ABITS> rcnt_inc;

    for (int i = 0; i < TRACE_TBL_SZ; i++) {
        v.trace_tbl[i].exec_cnt = r.trace_tbl[i].exec_cnt.read();
        v.trace_tbl[i].pc = r.trace_tbl[i].pc.read();
        v.trace_tbl[i].instr = r.trace_tbl[i].instr.read();
        v.trace_tbl[i].regactioncnt = r.trace_tbl[i].regactioncnt.read();
        v.trace_tbl[i].memactioncnt = r.trace_tbl[i].memactioncnt.read();
        for (int j = 0; j < TRACE_TBL_SZ; j++) {
            v.trace_tbl[i].regaction[j].waddr = r.trace_tbl[i].regaction[j].waddr.read();
            v.trace_tbl[i].regaction[j].wres = r.trace_tbl[i].regaction[j].wres.read();
        }
        for (int j = 0; j < TRACE_TBL_SZ; j++) {
            v.trace_tbl[i].memaction[j].store = r.trace_tbl[i].memaction[j].store.read();
            v.trace_tbl[i].memaction[j].size = r.trace_tbl[i].memaction[j].size.read();
            v.trace_tbl[i].memaction[j].mask = r.trace_tbl[i].memaction[j].mask.read();
            v.trace_tbl[i].memaction[j].memaddr = r.trace_tbl[i].memaction[j].memaddr.read();
            v.trace_tbl[i].memaction[j].data = r.trace_tbl[i].memaction[j].data.read();
            v.trace_tbl[i].memaction[j].regaddr = r.trace_tbl[i].memaction[j].regaddr.read();
            v.trace_tbl[i].memaction[j].complete = r.trace_tbl[i].memaction[j].complete.read();
            v.trace_tbl[i].memaction[j].sc_release = r.trace_tbl[i].memaction[j].sc_release.read();
            v.trace_tbl[i].memaction[j].ignored = r.trace_tbl[i].memaction[j].ignored.read();
        }
        v.trace_tbl[i].completed = r.trace_tbl[i].completed.read();
    }
    v.tr_wcnt = r.tr_wcnt.read();
    v.tr_rcnt = r.tr_rcnt.read();
    v.tr_total = r.tr_total.read();
    v.tr_opened = r.tr_opened.read();
    wcnt = 0;
    xcnt = 0;
    rcnt = 0;
    regcnt = 0;
    memcnt = 0;
    mskoff = 0;
    mask = 0;
    tr_wcnt_nxt = 0;
    checked = 0;
    entry_valid = 0;
    rcnt_inc = 0;

    wcnt = r.tr_wcnt.read().to_int();
    rcnt = r.tr_rcnt.read().to_int();
    regcnt = r.trace_tbl[wcnt].regactioncnt.read().to_int();
    memcnt = r.trace_tbl[wcnt].memactioncnt.read().to_int();

    tr_wcnt_nxt = (r.tr_wcnt.read() + 1);
    if (i_e_valid.read() == 1) {
        v.trace_tbl[wcnt].exec_cnt = (i_dbg_executed_cnt.read() + 1);
        v.trace_tbl[wcnt].pc = i_e_pc.read();
        v.trace_tbl[wcnt].instr = i_e_instr.read();

        v.tr_wcnt = (r.tr_wcnt.read() + 1);
        // Clear next element:
        v.trace_tbl[tr_wcnt_nxt].regactioncnt = 0;
        v.trace_tbl[tr_wcnt_nxt].memactioncnt = 0;
        v.trace_tbl[tr_wcnt_nxt].completed = 0;
    }

    if ((i_e_memop_valid.read() == 1) && (i_m_memop_ready.read() == 1)) {
        v.trace_tbl[wcnt].memactioncnt = (r.trace_tbl[wcnt].memactioncnt.read() + 1);
        v.trace_tbl[wcnt].memaction[memcnt].store = i_e_memop_type.read()[MemopType_Store];
        v.trace_tbl[wcnt].memaction[memcnt].memaddr = i_e_memop_addr.read();
        v.trace_tbl[wcnt].memaction[memcnt].size = i_e_memop_size.read();
        // Compute and save mask bit
        mskoff = 0;
        mask = ~0ull;
        mskoff[i_e_memop_size.read().to_int()] = 1;
        mskoff = (mskoff << 3);
        if (mskoff < 64) {
            mask = 0;
            mask[mskoff.to_int()] = 1;
            mask = (mask - 1);
        }
        v.trace_tbl[wcnt].memaction[memcnt].mask = mask;
        v.trace_tbl[wcnt].memaction[memcnt].data = (i_e_memop_wdata.read() & mask);
        v.trace_tbl[wcnt].memaction[memcnt].regaddr = i_e_waddr.read();
        v.trace_tbl[wcnt].memaction[memcnt].ignored = 0;
        v.trace_tbl[wcnt].memaction[memcnt].complete = 0;
        if ((i_e_waddr.read().or_reduce() == 0)
                || ((i_e_memop_type.read()[MemopType_Store] == 1)
                        && (i_e_memop_type.read()[MemopType_Release] == 0))) {
            v.trace_tbl[wcnt].memaction[memcnt].complete = 1;
        }
        v.trace_tbl[wcnt].memaction[memcnt].sc_release = i_e_memop_type.read()[MemopType_Release];
    }

    if (i_e_wena.read() == 1) {
        // Direct register writting if it is not a Load operation
        v.trace_tbl[wcnt].regactioncnt = (r.trace_tbl[wcnt].regactioncnt.read() + 1);
        v.trace_tbl[wcnt].regaction[regcnt].waddr = i_e_waddr.read();
        v.trace_tbl[wcnt].regaction[regcnt].wres = i_e_wdata.read();
    } else if (i_m_wena.read() == 1) {
        // Update current rd memory action (memory operations are strictly ordered)
        // Find next instruction with the unfinished memory operation
        checked = 0;
        rcnt_inc = r.tr_rcnt.read();
        while ((checked == 0) && (rcnt_inc != r.tr_wcnt.read())) {
            xcnt = rcnt_inc.to_int();
            regcnt = r.trace_tbl[xcnt].regactioncnt.read().to_int();
            for (int i = 0; i < r.trace_tbl[xcnt].memactioncnt.read().to_int(); i++) {
                if ((checked == 0) && (r.trace_tbl[xcnt].memaction[i].complete.read() == 0)) {
                    checked = 1;
                    v.trace_tbl[xcnt].memaction[i].complete = 1;
                    v.trace_tbl[xcnt].memaction[i].ignored = i_reg_ignored.read();
                    if (r.trace_tbl[xcnt].memaction[i].sc_release.read() == 1) {
                        if (i_m_wdata.read() == 1) {
                            v.trace_tbl[xcnt].memaction[i].ignored = 1;
                        }
                        // do not re-write stored value by returning error status
                    } else {
                        v.trace_tbl[xcnt].memaction[i].data = i_m_wdata.read();
                    }

                    if (i_reg_ignored.read() == 0) {
                        v.trace_tbl[xcnt].regaction[regcnt].waddr = i_m_waddr.read();
                        v.trace_tbl[xcnt].regaction[regcnt].wres = i_m_wdata.read();
                        v.trace_tbl[xcnt].regactioncnt = (regcnt + 1);
                    }
                }
            }
            if (checked == 0) {
                rcnt_inc = (rcnt_inc + 1);
            }
        }
    }

    // check instruction data completness
    entry_valid = 1;
    rcnt_inc = r.tr_rcnt.read();
    outstr = "";
    while ((entry_valid == 1) && (rcnt_inc != r.tr_wcnt.read())) {
        for (int i = 0; i < r.trace_tbl[rcnt_inc].memactioncnt.read().to_int(); i++) {
            if (r.trace_tbl[rcnt_inc].memaction[i].complete.read() == 0) {
                entry_valid = 0;
            }
        }
        if (entry_valid == 1) {
            tracestr = TraceOutput(rcnt_inc);
            outstr += tracestr;
            rcnt_inc = (rcnt_inc + 1);
        }
    }
    v.tr_rcnt = rcnt_inc;

    if ((!async_reset_) && (i_nrst.read() == 0)) {
        Tracer_r_reset(v);
    }
}

void Tracer::traceout() {
    if (outstr != "") {
        fwrite(outstr.c_str(), 1, outstr.size(), fl);
    }
    outstr = "";
}

void Tracer::registers() {
    if ((async_reset_ == 1) && (i_nrst.read() == 0)) {
        Tracer_r_reset(r);
    } else {
        for (int i = 0; i < TRACE_TBL_SZ; i++) {
            r.trace_tbl[i].exec_cnt = v.trace_tbl[i].exec_cnt.read();
            r.trace_tbl[i].pc = v.trace_tbl[i].pc.read();
            r.trace_tbl[i].instr = v.trace_tbl[i].instr.read();
            r.trace_tbl[i].regactioncnt = v.trace_tbl[i].regactioncnt.read();
            r.trace_tbl[i].memactioncnt = v.trace_tbl[i].memactioncnt.read();
            for (int j = 0; j < TRACE_TBL_SZ; j++) {
                r.trace_tbl[i].regaction[j].waddr = v.trace_tbl[i].regaction[j].waddr.read();
                r.trace_tbl[i].regaction[j].wres = v.trace_tbl[i].regaction[j].wres.read();
            }
            for (int j = 0; j < TRACE_TBL_SZ; j++) {
                r.trace_tbl[i].memaction[j].store = v.trace_tbl[i].memaction[j].store.read();
                r.trace_tbl[i].memaction[j].size = v.trace_tbl[i].memaction[j].size.read();
                r.trace_tbl[i].memaction[j].mask = v.trace_tbl[i].memaction[j].mask.read();
                r.trace_tbl[i].memaction[j].memaddr = v.trace_tbl[i].memaction[j].memaddr.read();
                r.trace_tbl[i].memaction[j].data = v.trace_tbl[i].memaction[j].data.read();
                r.trace_tbl[i].memaction[j].regaddr = v.trace_tbl[i].memaction[j].regaddr.read();
                r.trace_tbl[i].memaction[j].complete = v.trace_tbl[i].memaction[j].complete.read();
                r.trace_tbl[i].memaction[j].sc_release = v.trace_tbl[i].memaction[j].sc_release.read();
                r.trace_tbl[i].memaction[j].ignored = v.trace_tbl[i].memaction[j].ignored.read();
            }
            r.trace_tbl[i].completed = v.trace_tbl[i].completed.read();
        }
        r.tr_wcnt = v.tr_wcnt.read();
        r.tr_rcnt = v.tr_rcnt.read();
        r.tr_total = v.tr_total.read();
        r.tr_opened = v.tr_opened.read();
    }
}

}  // namespace debugger

