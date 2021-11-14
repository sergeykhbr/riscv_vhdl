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
 */

#include "regibank.h"

namespace debugger {

RegIntBank::RegIntBank(sc_module_name name_, bool async_reset, bool fpu_ena) :
    sc_module(name_),
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
    o_sp("o_sp") {
    async_reset_ = async_reset;
    fpu_ena_ = fpu_ena;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_radr1;
    sensitive << i_radr2;
    sensitive << i_wena;
    sensitive << i_wtag;
    sensitive << i_wdata;
    sensitive << i_waddr;
    sensitive << i_inorder;
    sensitive << i_dport_ena;
    sensitive << i_dport_write;
    sensitive << i_dport_addr;
    sensitive << i_dport_wdata;
    sensitive << r.update;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
};

void RegIntBank::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_wena, i_wena.name());
        sc_trace(o_vcd, i_waddr, i_waddr.name());
        sc_trace(o_vcd, i_wdata, i_wdata.name());
        sc_trace(o_vcd, i_wtag, i_wtag.name());
        sc_trace(o_vcd, i_inorder, i_inorder.name());
        sc_trace(o_vcd, o_ignored, o_ignored.name());
        sc_trace(o_vcd, i_radr1, i_radr1.name());
        sc_trace(o_vcd, i_radr2, i_radr2.name());
        sc_trace(o_vcd, o_rdata1, o_rdata1.name());
        sc_trace(o_vcd, o_rdata2, o_rdata2.name());
        sc_trace(o_vcd, o_rtag1, o_rtag1.name());
        sc_trace(o_vcd, o_rtag2, o_rtag2.name());
        sc_trace(o_vcd, o_dport_rdata, o_dport_rdata.name());

        std::string pn(name());
        sc_trace(o_vcd, r.reg[Reg_s0].val, pn + ".s0");
        sc_trace(o_vcd, r.reg[Reg_s0].tag, pn + ".s0_tag");
        sc_trace(o_vcd, r.reg[Reg_s1].val, pn + ".s1");
        sc_trace(o_vcd, r.reg[Reg_s1].tag, pn + ".s1_tag");
        sc_trace(o_vcd, r.reg[Reg_a5].val, pn + ".a5");
        sc_trace(o_vcd, r.reg[Reg_a5].tag, pn + ".a5_tag");
    }
}

void RegIntBank::comb() {
    int int_daddr = i_dport_addr.read()(REG_MSB(), 0).to_int();
    int int_waddr = i_waddr.read()(REG_MSB(), 0).to_int();
    int int_radr1 = i_radr1.read()(REG_MSB(), 0).to_int();
    int int_radr2 = i_radr2.read()(REG_MSB(), 0).to_int();
    bool v_inordered;

    v = r;

    v_inordered = 0;
    sc_uint<CFG_REG_TAG_WITH> next_tag;

    next_tag = r.reg[int_waddr].tag + 1;
    if (next_tag == i_wtag.read()) {
        v_inordered = 1;
    }

    /** Debug port has lower priority to avoid system hangup due the tags error */
    if (i_wena.read() == 1 && i_waddr.read().or_reduce() == 1
        && (!i_inorder.read() || v_inordered)) {
        v.reg[int_waddr].val = i_wdata;
        v.reg[int_waddr].tag = i_wtag;
    } else if (i_dport_ena.read() && i_dport_write.read()) {
        if (i_dport_addr.read().or_reduce() == 1) {
            v.reg[int_daddr].val = i_dport_wdata;
        }
    }
    v.update = !r.update.read();

    if (!async_reset_ && !i_nrst.read()) {   
        v.reg[0].val = 0;
        v.reg[0].tag = 0;
        for (int i = 1; i < REGS_TOTAL; i++) {
            v.reg[i].val = 0xfeedface;
            v.reg[i].tag = 0;
        }
        v.update = 0;
    }

    o_ignored = i_wena && i_waddr.read().or_reduce() && i_inorder && !v_inordered;

    o_rdata1 = r.reg[int_radr1].val;
    o_rtag1 = r.reg[int_radr1].tag;
    o_rdata2 = r.reg[int_radr2].val;
    o_rtag2 = r.reg[int_radr2].tag;
    o_dport_rdata = r.reg[int_daddr].val;
    o_ra = r.reg[Reg_ra].val;
    o_sp = r.reg[Reg_sp].val;
}

void RegIntBank::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        r.reg[0].val = 0;
        r.reg[0].tag = 0;
        for (int i = 1; i < REGS_TOTAL; i++) {
            r.reg[i].val = 0xfeedface;
            r.reg[i].tag = 0;
        }
        r.update = 0;
    } else {
        r = v;
    }
}

}  // namespace debugger

