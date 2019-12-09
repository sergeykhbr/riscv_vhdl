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
#include <api_core.h>

namespace debugger {

ILru::ILru(sc_module_name name_) : sc_module(name_) ,
    i_init("i_init"),
    i_radr("i_radr"),
    i_wadr("i_wadr"),
    i_we("i_we"),
    i_lru("i_lru"),
    o_lru("o_lru") {

    SC_METHOD(comb);
    sensitive << i_init;
    sensitive << i_radr;
    sensitive << i_wadr;
    sensitive << i_we;
    sensitive << i_lru;
    sensitive << radr;
    sensitive << wb_tbl_rdata;

    SC_METHOD(registers);
    sensitive << i_clk.pos();
};

void ILru::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_init, i_init.name());
        sc_trace(o_vcd, i_radr, i_radr.name());
        sc_trace(o_vcd, i_wadr, i_wadr.name());
        sc_trace(o_vcd, i_we, i_we.name());
        sc_trace(o_vcd, i_lru, i_lru.name());
        sc_trace(o_vcd, o_lru, o_lru.name());

        std::string pn(name());
        sc_trace(o_vcd, wb_tbl_wdata, pn + ".wb_tbl_wdata");
    }
}

void ILru::comb() {
    sc_uint<8> vb_tbl_wdata;
    sc_uint<8> vb_tbl_rdata;
    bool v_we;

    vb_tbl_rdata = tbl[radr.read().to_int()];

    v_we = i_we.read();
    if (i_init.read() == 1) {
        v_we = 1;
        vb_tbl_wdata = 0xe4;        // 0x3, 0x2, 0x1, 0x0
    } else if (i_we.read()) {
        if (vb_tbl_rdata(7, 6) == i_lru.read()) {
            vb_tbl_wdata = vb_tbl_rdata;
        } else if (vb_tbl_rdata(5, 4) == i_lru.read()) {
            vb_tbl_wdata = (i_lru.read(),
                            vb_tbl_rdata(7, 6),
                            vb_tbl_rdata(3, 0));
        } else if (vb_tbl_rdata(3, 2) == i_lru.read()) {
            vb_tbl_wdata = (i_lru.read(),
                            vb_tbl_rdata(7, 4),
                            vb_tbl_rdata(1, 0));
        } else {
            vb_tbl_wdata = (i_lru.read(),
                            vb_tbl_rdata(7, 2));
        }
    }

    w_we = v_we;
    wb_tbl_wdata = vb_tbl_wdata;
    o_lru = vb_tbl_rdata(1, 0);
}

void ILru::registers() {
    radr = i_radr.read();
    wb_tbl_rdata = tbl[i_radr.read().to_int()];
    if (w_we.read() == 1) {
        tbl[i_wadr.read().to_int()] = wb_tbl_wdata;
    }
}

}  // namespace debugger

