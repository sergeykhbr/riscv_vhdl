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

#include "vip_sdcard_crc7.h"
#include "api_core.h"

namespace debugger {

vip_sdcard_crc7::vip_sdcard_crc7(sc_module_name name,
                                 bool async_reset)
    : sc_module(name),
    i_clk("i_clk"),
    i_nrst("i_nrst"),
    i_clear("i_clear"),
    i_next("i_next"),
    i_dat("i_dat"),
    o_crc7("o_crc7") {

    async_reset_ = async_reset;

    SC_METHOD(comb);
    sensitive << i_nrst;
    sensitive << i_clear;
    sensitive << i_next;
    sensitive << i_dat;
    sensitive << r.crc7;

    SC_METHOD(registers);
    sensitive << i_nrst;
    sensitive << i_clk.pos();
}

void vip_sdcard_crc7::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    std::string pn(name());
    if (o_vcd) {
        sc_trace(o_vcd, i_clear, i_clear.name());
        sc_trace(o_vcd, i_next, i_next.name());
        sc_trace(o_vcd, i_dat, i_dat.name());
        sc_trace(o_vcd, o_crc7, o_crc7.name());
        sc_trace(o_vcd, r.crc7, pn + ".r.crc7");
    }

}

void vip_sdcard_crc7::comb() {
    bool v_inv7;
    sc_uint<7> vb_crc7;

    v = r;
    v_inv7 = 0;
    vb_crc7 = 0;

    // CRC7 = x^7 + x^3 + 1
    // CMD0 -> 01 000000 0000..000000000000000000000000 1001010 1 -> 0x4A (0x95)
    // CMD17-> 01 010001 0000..000000000000000000000000 0101010 1 -> 0x2A (0x55)
    // CMD17<- 00 010001 0000..000000000010010000000000 0110011 1 -> 0x33 (0x67)
    v_inv7 = (r.crc7.read()[6] ^ i_dat.read());
    vb_crc7[6] = r.crc7.read()[5];
    vb_crc7[5] = r.crc7.read()[4];
    vb_crc7[4] = r.crc7.read()[3];
    vb_crc7[3] = (r.crc7.read()[2] ^ v_inv7);
    vb_crc7[2] = r.crc7.read()[1];
    vb_crc7[1] = r.crc7.read()[0];
    vb_crc7[0] = v_inv7;

    if (i_clear.read() == 1) {
        v.crc7 = 0;
    } else if (i_next.read() == 1) {
        v.crc7 = vb_crc7;
    }

    if ((!async_reset_) && (i_nrst.read() == 0)) {
        vip_sdcard_crc7_r_reset(v);
    }

    o_crc7 = vb_crc7;
}

void vip_sdcard_crc7::registers() {
    if ((async_reset_ == 1) && (i_nrst.read() == 0)) {
        vip_sdcard_crc7_r_reset(r);
    } else {
        r = v;
    }
}

}  // namespace debugger

