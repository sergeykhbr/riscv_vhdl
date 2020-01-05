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
    o_rhazard1("o_rhazard1"),
    i_radr2("i_radr2"),
    o_rdata2("o_rdata2"),
    o_rhazard2("o_rhazard2"),
    i_waddr("i_waddr"),
    i_wena("i_wena"),
    i_whazard("i_whazard"),
    i_wtag("i_wtag"),
    i_wdata("i_wdata"),
    o_wtag("o_wtag"),
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
    sensitive << i_whazard;
    sensitive << i_wtag;
    sensitive << i_wdata;
    sensitive << i_waddr;
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
        sc_trace(o_vcd, o_rdata1, o_rdata1.name());
        sc_trace(o_vcd, o_rdata2, o_rdata2.name());
        sc_trace(o_vcd, o_wtag, o_wtag.name());
        sc_trace(o_vcd, o_dport_rdata, o_dport_rdata.name());

        std::string pn(name());
        sc_trace(o_vcd, r.reg[5].val, pn + ".r4");
    }
}

void RegIntBank::comb() {
    int int_waddr = i_waddr.read()(REG_MSB(), 0).to_int();
    int int_radr1 = i_radr1.read()(REG_MSB(), 0).to_int();
    int int_radr2 = i_radr2.read()(REG_MSB(), 0).to_int();

    v = r;

    /** Debug port has higher priority. Collision must be controlled by SW */
    if (i_dport_ena.read() && i_dport_write.read()) {
        if (i_dport_addr.read() != 0) {
            v.reg[i_dport_addr.read().to_int()].val = i_dport_wdata;
            v.reg[int_waddr].hazard = 0;
        }
    } else if (i_wena.read() == 1 && i_waddr.read().or_reduce() == 1) {
        if (i_wtag.read() == r.reg[int_waddr].tag) {
            v.reg[int_waddr].hazard = i_whazard;
            v.reg[int_waddr].val = i_wdata;
            if (i_whazard.read() == 0) {
                v.reg[int_waddr].tag = r.reg[int_waddr].tag + 1;
            }
        }
    }
    v.update = !r.update.read();

    if (!async_reset_ && !i_nrst.read()) {   
        v.reg[0].hazard = 0;
        v.reg[0].val = 0;
        v.reg[0].tag = 0;
        for (int i = 1; i < REGS_TOTAL; i++) {
            v.reg[i].hazard = 0;
            v.reg[i].val = 0xfeedface;
            v.reg[i].tag = 0;
        }
        v.update = 0;
    }

    o_rdata1 = r.reg[int_radr1].val;
    o_rhazard1 = r.reg[int_radr1].hazard;
    o_rdata2 = r.reg[int_radr2].val;
    o_rhazard2 = r.reg[int_radr2].hazard;
    o_wtag = r.reg[int_waddr].tag;
    o_dport_rdata = r.reg[i_dport_addr.read().to_int()].val;
    o_ra = r.reg[Reg_ra].val;
    o_sp = r.reg[Reg_sp].val;
}

void RegIntBank::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        r.reg[0].hazard = 0;
        r.reg[0].val = 0;
        r.reg[0].tag = 0;
        for (int i = 1; i < REGS_TOTAL; i++) {
            r.reg[i].hazard = 0;
            r.reg[i].val = 0xfeedface;
            r.reg[i].tag = 0;
        }
        r.update = 0;
    } else {
        r = v;
    }
}

}  // namespace debugger

