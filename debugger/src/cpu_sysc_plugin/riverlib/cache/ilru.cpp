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

#include "ilru.h"

namespace debugger {

ILru::ILru(sc_module_name name_) : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_init;
    sensitive << i_adr;
    sensitive << i_we;
    sensitive << i_lru;
    sensitive << radr;
    sensitive << wb_tbl_rdata;

    SC_METHOD(registers);
    sensitive << i_clk.pos();
};

void ILru::comb() {
    sc_uint<8> vb_tbl_wdata;
    bool v_we;

    wb_tbl_rdata = tbl[radr.read()];

    v_we - i_we.read();
    if (i_init.read() == 1) {
        vb_tbl_wdata = 0xe4;        // 0x3, 0x2, 0x1, 0x0
    } else if (i_we.read() && wb_tbl_rdata.read()(7, 6) != i_lru.read()) {
        vb_tbl_wdata = (i_lru.read(), wb_tbl_rdata.read()(7, 2));
    } else {
        vb_tbl_wdata = wb_tbl_rdata.read();
    }

    w_we = v_we;
    wb_tbl_wdata = vb_tbl_wdata;
    o_lru = wb_tbl_rdata.read()(1, 0);
}

void ILru::registers() {
    radr = i_adr.read();
    //wb_tbl_rdata = tbl[i_adr.read()];
    if (w_we.read() == 1) {
        tbl[i_adr.read()] = wb_tbl_wdata;
    }
}

}  // namespace debugger

