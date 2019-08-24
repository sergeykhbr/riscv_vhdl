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

#include "regfbank.h"

namespace debugger {

RegFloatBank::RegFloatBank(sc_module_name name_, bool async_reset) :
    sc_module(name_),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_radr1("i_radr1"),
    o_rdata1("o_rdata1"),
    i_radr2("i_radr2"),
    o_rdata2("o_rdata2"),
    i_waddr("i_waddr"),
    i_wena("i_wena"),
    i_wdata("i_wdata"),
    i_dport_addr("i_dport_addr"),
    i_dport_ena("i_dport_ena"),
    i_dport_write("i_dport_write"),
    i_dport_wdata("i_dport_wdata"),
    o_dport_rdata("o_dport_rdata") {
    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_radr1;
    sensitive << i_radr2;
    sensitive << i_wena;
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

void RegFloatBank::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_wena, i_wena.name());
        sc_trace(o_vcd, i_waddr, i_waddr.name());
        sc_trace(o_vcd, i_wdata, i_wdata.name());
        sc_trace(o_vcd, o_rdata1, o_rdata1.name());
        sc_trace(o_vcd, o_rdata2, o_rdata2.name());
        sc_trace(o_vcd, o_dport_rdata, o_dport_rdata.name());

        std::string pn(name());
        sc_trace(o_vcd, r.mem[0], pn + ".r_mem0");
        sc_trace(o_vcd, r.mem[1], pn + ".r_mem1");
        sc_trace(o_vcd, r.mem[2], pn + ".r_mem2");
        sc_trace(o_vcd, r.mem[3], pn + ".r_mem3");
        sc_trace(o_vcd, r.mem[14], pn + ".r_fa4");
        sc_trace(o_vcd, r.mem[15], pn + ".r_fa5");
    }
}

void RegFloatBank::comb() {
    v = r;

    /** Debug port has higher priority. Collision must be controlled by SW */
    if (i_dport_ena.read() && i_dport_write.read()) {
        if (i_dport_addr.read() != 0) {
            v.mem[i_dport_addr.read()] = i_dport_wdata;
        }
    } else if (i_wena.read() && i_waddr.read()[5] == 1) {
        v.mem[i_waddr.read()(4, 0)] = i_wdata;
    }
    /** v.mem[] is not a signals, so use update register to trigger process */
    v.update = !r.update.read();

    if (!async_reset_ && !i_nrst.read()) {   
        for (int i = 0; i < RegFpu_Total; i++) {
            v.mem[i] = 0xfeedface;
        }
        v.update = 0;
    }

    o_rdata1 = r.mem[i_radr1.read()(4, 0)];
    o_rdata2 = r.mem[i_radr2.read()(4, 0)];
    o_dport_rdata = r.mem[i_dport_addr.read()];
}

void RegFloatBank::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        for (int i = 0; i < RegFpu_Total; i++) {
            r.mem[i] = 0xfeedface;
        }
        r.update = 0;
    } else {
        r = v;
    }
}

}  // namespace debugger

