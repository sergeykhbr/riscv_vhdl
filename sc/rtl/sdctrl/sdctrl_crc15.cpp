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

#include "sdctrl_crc15.h"
#include "api_core.h"

namespace debugger {

sdctrl_crc15::sdctrl_crc15(sc_module_name name,
                           bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_clear("i_clear"),
    i_next("i_next"),
    i_dat("i_dat"),
    o_crc15("o_crc15") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_clear;
    sensitive << i_next;
    sensitive << i_dat;
    sensitive << r.crc15;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void sdctrl_crc15::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_clear, i_clear.name());
        sc_trace(o_vcd, i_next, i_next.name());
        sc_trace(o_vcd, i_dat, i_dat.name());
        sc_trace(o_vcd, o_crc15, o_crc15.name());
        sc_trace(o_vcd, r.crc15, pn + ".r_crc15");
    }

}

void sdctrl_crc15::comb() {
    bool v_inv15_0;
    sc_uint<15> vb_crc15_0;

    v_inv15_0 = 0;
    vb_crc15_0 = 0;

    v = r;

    // CRC15 = x^16 + x^12 + x^5 + 1
    v_inv15_0 = (r.crc15.read()[14] ^ i_dat.read());
    vb_crc15_0[14] = r.crc15.read()[13];
    vb_crc15_0[13] = r.crc15.read()[12];
    vb_crc15_0[12] = (r.crc15.read()[11] ^ v_inv15_0);
    vb_crc15_0[11] = r.crc15.read()[10];
    vb_crc15_0[10] = r.crc15.read()[9];
    vb_crc15_0[9] = r.crc15.read()[8];
    vb_crc15_0[8] = r.crc15.read()[7];
    vb_crc15_0[7] = r.crc15.read()[6];
    vb_crc15_0[6] = r.crc15.read()[5];
    vb_crc15_0[5] = (r.crc15.read()[4] ^ v_inv15_0);
    vb_crc15_0[4] = r.crc15.read()[3];
    vb_crc15_0[3] = r.crc15.read()[2];
    vb_crc15_0[2] = r.crc15.read()[1];
    vb_crc15_0[1] = r.crc15.read()[0];
    vb_crc15_0[0] = v_inv15_0;

    if (i_clear.read() == 1) {
        v.crc15 = 0;
    } else if (i_next.read() == 1) {
        v.crc15 = vb_crc15_0;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        sdctrl_crc15_r_reset(v);
    }

    o_crc15 = r.crc15;
}

void sdctrl_crc15::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        sdctrl_crc15_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

