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
#pragma once

#include <systemc.h>
#include "api_core.h"

namespace debugger {

template<int abits = 6,                                     // Cache line address bus (usually 6..8)
         int waybits = 2>                                   // Number of way bitwidth (=2 for 4 ways cache)
SC_MODULE(lrunway) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_init;
    sc_in<sc_uint<abits>> i_raddr;
    sc_in<sc_uint<abits>> i_waddr;
    sc_in<bool> i_up;
    sc_in<bool> i_down;
    sc_in<sc_uint<waybits>> i_lru;
    sc_out<sc_uint<waybits>> o_lru;

    void comb();
    void registers();

    SC_HAS_PROCESS(lrunway);

    lrunway(sc_module_name name);


 private:
    static const int LINES_TOTAL = (1 << abits);
    static const int WAYS_TOTAL = (1 << waybits);
    static const int LINE_WIDTH = (WAYS_TOTAL * waybits);

    struct lrunway_registers {
        sc_signal<sc_uint<abits>> radr;
        sc_signal<sc_uint<LINE_WIDTH>> mem[LINES_TOTAL];
    } v, r;


};

template<int abits, int waybits>
lrunway<abits, waybits>::lrunway(sc_module_name name)
    : sc_module(name),
    i_clk("i_clk"),
    i_init("i_init"),
    i_raddr("i_raddr"),
    i_waddr("i_waddr"),
    i_up("i_up"),
    i_down("i_down"),
    i_lru("i_lru"),
    o_lru("o_lru") {


    SC_METHOD(comb);
    sensitive << i_init;
    sensitive << i_raddr;
    sensitive << i_waddr;
    sensitive << i_up;
    sensitive << i_down;
    sensitive << i_lru;
    sensitive << r.radr;
    for (int i = 0; i < LINES_TOTAL; i++) {
        sensitive << r.mem[i];
    }

    SC_METHOD(registers);
    sensitive << i_clk.pos();
}

template<int abits, int waybits>
void lrunway<abits, waybits>::comb() {
    sc_uint<LINE_WIDTH> wb_tbl_rdata;
    sc_uint<abits> vb_tbl_wadr;
    sc_uint<LINE_WIDTH> vb_tbl_wdata_init;
    sc_uint<LINE_WIDTH> vb_tbl_wdata_up;
    sc_uint<LINE_WIDTH> vb_tbl_wdata_down;
    sc_uint<LINE_WIDTH> vb_tbl_wdata;
    bool v_we;
    bool shift_ena_up;
    bool shift_ena_down;

    wb_tbl_rdata = 0;
    vb_tbl_wadr = 0;
    vb_tbl_wdata_init = 0;
    vb_tbl_wdata_up = 0;
    vb_tbl_wdata_down = 0;
    vb_tbl_wdata = 0;
    v_we = 0;
    shift_ena_up = 0;
    shift_ena_down = 0;

    v.radr = r.radr;
    for (int i = 0; i < LINES_TOTAL; i++) {
        v.mem[i] = r.mem[i];
    }

    v.radr = i_raddr;
    wb_tbl_rdata = r.mem[r.radr.read().to_int()];

    v_we = (i_up || i_down || i_init);

    // init table value
    for (int i = 0; i < WAYS_TOTAL; i++) {
        vb_tbl_wdata_init((i * waybits) + waybits- 1, (i * waybits)) = i;
    }

    // LRU next value, last used goes on top
    vb_tbl_wdata_up = wb_tbl_rdata;
    if (wb_tbl_rdata((LINE_WIDTH - waybits) + waybits - 1, (LINE_WIDTH - waybits)) != i_lru.read()) {
        vb_tbl_wdata_up((LINE_WIDTH - waybits) + waybits- 1, (LINE_WIDTH - waybits)) = i_lru;
        shift_ena_up = 1;

        for (int i = (WAYS_TOTAL - 2); i >= 0; i--) {
            if (shift_ena_up == 1) {
                vb_tbl_wdata_up((i * waybits) + waybits- 1, (i * waybits)) = wb_tbl_rdata(((i + 1) * waybits) + waybits - 1, ((i + 1) * waybits));
                if (wb_tbl_rdata((i * waybits) + waybits - 1, (i * waybits)) == i_lru.read()) {
                    shift_ena_up = 0;
                }
            }
        }
    }

    // LRU next value when invalidate, marked as 'invalid' goes down
    vb_tbl_wdata_down = wb_tbl_rdata;
    if (wb_tbl_rdata((waybits - 1), 0) != i_lru.read()) {
        vb_tbl_wdata_down((waybits - 1), 0) = i_lru;
        shift_ena_down = 1;

        for (int i = 1; i < WAYS_TOTAL; i++) {
            if (shift_ena_down == 1) {
                vb_tbl_wdata_down((i * waybits) + waybits- 1, (i * waybits)) = wb_tbl_rdata(((i - 1) * waybits) + waybits - 1, ((i - 1) * waybits));
                if (wb_tbl_rdata((i * waybits) + waybits - 1, (i * waybits)) == i_lru.read()) {
                    shift_ena_down = 0;
                }
            }
        }
    }

    if (i_init.read() == 1) {
        vb_tbl_wdata = vb_tbl_wdata_init;
    } else if (i_up.read() == 1) {
        vb_tbl_wdata = vb_tbl_wdata_up;
    } else if (i_down.read() == 1) {
        vb_tbl_wdata = vb_tbl_wdata_down;
    } else {
        vb_tbl_wdata = 0;
    }

    if (v_we == 1) {
        v.mem[i_waddr.read().to_int()] = vb_tbl_wdata;
    }
    o_lru = wb_tbl_rdata((waybits - 1), 0);
}

template<int abits, int waybits>
void lrunway<abits, waybits>::registers() {
    r.radr = v.radr;
    for (int i = 0; i < LINES_TOTAL; i++) {
        r.mem[i] = v.mem[i];
    }
}

}  // namespace debugger

