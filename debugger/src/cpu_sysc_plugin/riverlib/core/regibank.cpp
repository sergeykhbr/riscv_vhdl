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

#include "regibank.h"
#include "api_core.h"

namespace debugger {

RegIntBank::RegIntBank(sc_module_name name,
                       bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_radr1("i_radr1"),
    o_rdata1("o_rdata1"),
    o_rtag1("o_rtag1"),
    i_radr2("i_radr2"),
    o_rdata2("o_rdata2"),
    o_rtag2("o_rtag2"),
    i_waddr("i_waddr"),
    i_wena("i_wena"),
    i_wtag("i_wtag"),
    i_wdata("i_wdata"),
    i_inorder("i_inorder"),
    o_ignored("o_ignored"),
    i_dport_addr("i_dport_addr"),
    i_dport_ena("i_dport_ena"),
    i_dport_write("i_dport_write"),
    i_dport_wdata("i_dport_wdata"),
    o_dport_rdata("o_dport_rdata"),
    o_ra("o_ra"),
    o_sp("o_sp"),
    o_gp("o_gp"),
    o_tp("o_tp"),
    o_t0("o_t0"),
    o_t1("o_t1"),
    o_t2("o_t2"),
    o_fp("o_fp"),
    o_s1("o_s1"),
    o_a0("o_a0"),
    o_a1("o_a1"),
    o_a2("o_a2"),
    o_a3("o_a3"),
    o_a4("o_a4"),
    o_a5("o_a5"),
    o_a6("o_a6"),
    o_a7("o_a7"),
    o_s2("o_s2"),
    o_s3("o_s3"),
    o_s4("o_s4"),
    o_s5("o_s5"),
    o_s6("o_s6"),
    o_s7("o_s7"),
    o_s8("o_s8"),
    o_s9("o_s9"),
    o_s10("o_s10"),
    o_s11("o_s11"),
    o_t3("o_t3"),
    o_t4("o_t4"),
    o_t5("o_t5"),
    o_t6("o_t6") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_radr1;
    sensitive << i_radr2;
    sensitive << i_waddr;
    sensitive << i_wena;
    sensitive << i_wtag;
    sensitive << i_wdata;
    sensitive << i_inorder;
    sensitive << i_dport_addr;
    sensitive << i_dport_ena;
    sensitive << i_dport_write;
    sensitive << i_dport_wdata;
    for (int i = 0; i < REGS_TOTAL; i++) {
        sensitive << r.arr[i].val;
        sensitive << r.arr[i].tag;
    }

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void RegIntBank::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_radr1, i_radr1.name());
        sc_trace(o_vcd, o_rdata1, o_rdata1.name());
        sc_trace(o_vcd, o_rtag1, o_rtag1.name());
        sc_trace(o_vcd, i_radr2, i_radr2.name());
        sc_trace(o_vcd, o_rdata2, o_rdata2.name());
        sc_trace(o_vcd, o_rtag2, o_rtag2.name());
        sc_trace(o_vcd, i_waddr, i_waddr.name());
        sc_trace(o_vcd, i_wena, i_wena.name());
        sc_trace(o_vcd, i_wtag, i_wtag.name());
        sc_trace(o_vcd, i_wdata, i_wdata.name());
        sc_trace(o_vcd, i_inorder, i_inorder.name());
        sc_trace(o_vcd, o_ignored, o_ignored.name());
        sc_trace(o_vcd, i_dport_addr, i_dport_addr.name());
        sc_trace(o_vcd, i_dport_ena, i_dport_ena.name());
        sc_trace(o_vcd, i_dport_write, i_dport_write.name());
        sc_trace(o_vcd, i_dport_wdata, i_dport_wdata.name());
        sc_trace(o_vcd, o_dport_rdata, o_dport_rdata.name());
        sc_trace(o_vcd, o_ra, o_ra.name());
        sc_trace(o_vcd, o_sp, o_sp.name());
        sc_trace(o_vcd, o_gp, o_gp.name());
        sc_trace(o_vcd, o_tp, o_tp.name());
        sc_trace(o_vcd, o_t0, o_t0.name());
        sc_trace(o_vcd, o_t1, o_t1.name());
        sc_trace(o_vcd, o_t2, o_t2.name());
        sc_trace(o_vcd, o_fp, o_fp.name());
        sc_trace(o_vcd, o_s1, o_s1.name());
        sc_trace(o_vcd, o_a0, o_a0.name());
        sc_trace(o_vcd, o_a1, o_a1.name());
        sc_trace(o_vcd, o_a2, o_a2.name());
        sc_trace(o_vcd, o_a3, o_a3.name());
        sc_trace(o_vcd, o_a4, o_a4.name());
        sc_trace(o_vcd, o_a5, o_a5.name());
        sc_trace(o_vcd, o_a6, o_a6.name());
        sc_trace(o_vcd, o_a7, o_a7.name());
        sc_trace(o_vcd, o_s2, o_s2.name());
        sc_trace(o_vcd, o_s3, o_s3.name());
        sc_trace(o_vcd, o_s4, o_s4.name());
        sc_trace(o_vcd, o_s5, o_s5.name());
        sc_trace(o_vcd, o_s6, o_s6.name());
        sc_trace(o_vcd, o_s7, o_s7.name());
        sc_trace(o_vcd, o_s8, o_s8.name());
        sc_trace(o_vcd, o_s9, o_s9.name());
        sc_trace(o_vcd, o_s10, o_s10.name());
        sc_trace(o_vcd, o_s11, o_s11.name());
        sc_trace(o_vcd, o_t3, o_t3.name());
        sc_trace(o_vcd, o_t4, o_t4.name());
        sc_trace(o_vcd, o_t5, o_t5.name());
        sc_trace(o_vcd, o_t6, o_t6.name());
        for (int i = 0; i < REGS_TOTAL; i++) {
            char tstr[1024];
            RISCV_sprintf(tstr, sizeof(tstr), "%s.r_arr%d_val", pn.c_str(), i);
            sc_trace(o_vcd, r.arr[i].val, tstr);
            RISCV_sprintf(tstr, sizeof(tstr), "%s.r_arr%d_tag", pn.c_str(), i);
            sc_trace(o_vcd, r.arr[i].tag, tstr);
        }
    }

}

void RegIntBank::comb() {
    int int_daddr;
    int int_waddr;
    int int_radr1;
    int int_radr2;
    bool v_inordered;
    sc_uint<CFG_REG_TAG_WIDTH> next_tag;

    int_daddr = 0;
    int_waddr = 0;
    int_radr1 = 0;
    int_radr2 = 0;
    v_inordered = 0;
    next_tag = 0;

    for (int i = 0; i < REGS_TOTAL; i++) {
        v.arr[i].val = r.arr[i].val;
        v.arr[i].tag = r.arr[i].tag;
    }

    int_daddr = i_dport_addr.read().to_int();
    int_waddr = i_waddr.read().to_int();
    int_radr1 = i_radr1.read().to_int();
    int_radr2 = i_radr2.read().to_int();

    next_tag = (r.arr[int_waddr].tag.read() + 1);
    if (next_tag == i_wtag.read()) {
        v_inordered = 1;
    }

    // Debug port has lower priority to avoid system hangup due the tags error
    if ((i_wena.read() == 1) && (i_waddr.read().or_reduce() == 1) && (((!i_inorder) || v_inordered) == 1)) {
        v.arr[int_waddr].val = i_wdata;
        v.arr[int_waddr].tag = i_wtag;
    } else if ((i_dport_ena && i_dport_write) == 1) {
        if (i_dport_addr.read().or_reduce() == 1) {
            v.arr[int_daddr].val = i_dport_wdata;
        }
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        for (int i = 0; i < REGS_TOTAL; i++) {
            v.arr[i].val = 0ull;
            v.arr[i].tag = 0;
        }
    }

    o_ignored = (i_wena && i_waddr.read().or_reduce() && i_inorder && (!v_inordered));
    o_rdata1 = r.arr[int_radr1].val;
    o_rtag1 = r.arr[int_radr1].tag;
    o_rdata2 = r.arr[int_radr2].val;
    o_rtag2 = r.arr[int_radr2].tag;
    o_dport_rdata = r.arr[int_daddr].val;
    o_ra = r.arr[REG_RA].val;
    o_sp = r.arr[REG_SP].val;
    o_gp = r.arr[REG_GP].val;
    o_tp = r.arr[REG_TP].val;
    o_t0 = r.arr[REG_T0].val;
    o_t1 = r.arr[REG_T1].val;
    o_t2 = r.arr[REG_T2].val;
    o_fp = r.arr[REG_S0].val;
    o_s1 = r.arr[REG_S1].val;
    o_a0 = r.arr[REG_A0].val;
    o_a1 = r.arr[REG_A1].val;
    o_a2 = r.arr[REG_A2].val;
    o_a3 = r.arr[REG_A3].val;
    o_a4 = r.arr[REG_A4].val;
    o_a5 = r.arr[REG_A5].val;
    o_a6 = r.arr[REG_A6].val;
    o_a7 = r.arr[REG_A7].val;
    o_s2 = r.arr[REG_S2].val;
    o_s3 = r.arr[REG_S3].val;
    o_s4 = r.arr[REG_S4].val;
    o_s5 = r.arr[REG_S5].val;
    o_s6 = r.arr[REG_S6].val;
    o_s7 = r.arr[REG_S7].val;
    o_s8 = r.arr[REG_S8].val;
    o_s9 = r.arr[REG_S9].val;
    o_s10 = r.arr[REG_S10].val;
    o_s11 = r.arr[REG_S11].val;
    o_t3 = r.arr[REG_T3].val;
    o_t4 = r.arr[REG_T4].val;
    o_t5 = r.arr[REG_T5].val;
    o_t6 = r.arr[REG_T6].val;
}

void RegIntBank::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        for (int i = 0; i < REGS_TOTAL; i++) {
            r.arr[i].val = 0ull;
            r.arr[i].tag = 0;
        }
    } else {
        for (int i = 0; i < REGS_TOTAL; i++) {
            r.arr[i].val = v.arr[i].val;
            r.arr[i].tag = v.arr[i].tag;
        }
    }
}

}  // namespace debugger

