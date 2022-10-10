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

#include "pma.h"
#include "api_core.h"

namespace debugger {

PMA::PMA(sc_module_name name,
         bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_iaddr("i_iaddr"),
    i_daddr("i_daddr"),
    o_icached("o_icached"),
    o_dcached("o_dcached") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_iaddr;
    sensitive << i_daddr;
    sensitive << r.icached;
    sensitive << r.dcached;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void PMA::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_iaddr, i_iaddr.name());
        sc_trace(o_vcd, i_daddr, i_daddr.name());
        sc_trace(o_vcd, o_icached, o_icached.name());
        sc_trace(o_vcd, o_dcached, o_dcached.name());
        sc_trace(o_vcd, r.icached, pn + ".r_icached");
        sc_trace(o_vcd, r.dcached, pn + ".r_dcached");
    }

}

void PMA::comb() {
    bool v_icached;
    bool v_dcached;

    v_icached = 0;
    v_dcached = 0;

    v = r;

    v_icached = 1;
    if ((i_iaddr.read() & (~CLINT_MASK)) == CLINT_BAR) {
        v_icached = 0;
    } else if ((i_iaddr.read() & (~PLIC_MASK)) == PLIC_BAR) {
        v_icached = 0;
    } else if ((i_iaddr.read() & (~IO1_MASK)) == IO1_BAR) {
        v_icached = 0;
    }

    v_dcached = 1;
    if ((i_daddr.read() & (~CLINT_MASK)) == CLINT_BAR) {
        v_dcached = 0;
    } else if ((i_daddr.read() & (~PLIC_MASK)) == PLIC_BAR) {
        v_dcached = 0;
    } else if ((i_daddr.read() & (~IO1_MASK)) == IO1_BAR) {
        v_dcached = 0;
    }

    v.icached = v_icached;
    v.dcached = v_dcached;

    if (!async_reset_ && i_nrst.read() == 0) {
        PMA_r_reset(v);
    }

    o_icached = v_icached;
    o_dcached = v_dcached;
}

void PMA::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        PMA_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

