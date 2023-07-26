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

#include "sdctrl_crc16.h"
#include "api_core.h"

namespace debugger {

sdctrl_crc16::sdctrl_crc16(sc_module_name name,
                           bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_clear("i_clear"),
    i_next("i_next"),
    i_dat("i_dat"),
    o_crc16("o_crc16") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_clear;
    sensitive << i_next;
    sensitive << i_dat;
    sensitive << r.crc16;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void sdctrl_crc16::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_clear, i_clear.name());
        sc_trace(o_vcd, i_next, i_next.name());
        sc_trace(o_vcd, i_dat, i_dat.name());
        sc_trace(o_vcd, o_crc16, o_crc16.name());
        sc_trace(o_vcd, r.crc16, pn + ".r_crc16");
    }

}

void sdctrl_crc16::comb() {
    bool v_inv16;
    sc_uint<16> vb_crc16;

    v_inv16 = 0;
    vb_crc16 = 0;

    v = r;

    // CRC16 = x^16 + x^12 + x^5 + 1
    v_inv16 = (r.crc16.read()[15] ^ i_dat.read()[3]);
    vb_crc16[15] = r.crc16.read()[14];
    vb_crc16[14] = r.crc16.read()[13];
    vb_crc16[13] = r.crc16.read()[12];
    vb_crc16[12] = (r.crc16.read()[11] ^ v_inv16);
    vb_crc16[11] = r.crc16.read()[10];
    vb_crc16[10] = r.crc16.read()[9];
    vb_crc16[9] = r.crc16.read()[8];
    vb_crc16[8] = r.crc16.read()[7];
    vb_crc16[7] = r.crc16.read()[6];
    vb_crc16[6] = r.crc16.read()[5];
    vb_crc16[5] = (r.crc16.read()[4] ^ v_inv16);
    vb_crc16[4] = r.crc16.read()[3];
    vb_crc16[3] = r.crc16.read()[2];
    vb_crc16[2] = r.crc16.read()[1];
    vb_crc16[1] = r.crc16.read()[0];
    vb_crc16[0] = v_inv16;

    if (i_clear.read() == 1) {
        v.crc16 = 0;
    } else if (i_next.read() == 1) {
        v.crc16 = vb_crc16;
    }

    if (!async_reset_ && i_nrst.read() == 0) {
        sdctrl_crc16_r_reset(v);
    }

    o_crc16 = r.crc16;
}

void sdctrl_crc16::registers() {
    if (async_reset_ && i_nrst.read() == 0) {
        sdctrl_crc16_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

