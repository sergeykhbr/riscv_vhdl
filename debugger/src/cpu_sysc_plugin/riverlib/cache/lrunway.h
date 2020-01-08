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

#ifndef __DEBUGGER_RIVERLIB_CACHE_MEM_LRUNWAY_H__
#define __DEBUGGER_RIVERLIB_CACHE_MEM_LRUNWAY_H__

#include <systemc.h>

namespace debugger {
/**
    abits = Cache lines adress bus (usually 6..8)
    waybits = Number of way bitwidth (=2 for 4 ways cache)
*/
template <int abits, int waybits>
SC_MODULE(lrunway) {
    sc_in<bool> i_clk;
    sc_in<bool> i_flush;
    sc_in<sc_uint<abits>> i_addr;
    sc_in<bool> i_we;
    sc_in<sc_uint<waybits>> i_lru;
    sc_out<sc_uint<waybits>> o_lru;

    void comb();
    void registers();

    SC_HAS_PROCESS(lrunway);

    lrunway(sc_module_name name_) : sc_module(name_),
        i_clk("i_clk"),
        i_flush("i_flush"),
        i_addr("i_addr"),
        i_we("i_we"),
        i_lru("i_lru"),
        o_lru("o_lru") {

        SC_METHOD(comb);
        sensitive << i_flush;
        sensitive << i_addr;
        sensitive << i_we;
        sensitive << i_lru;
        sensitive << radr;
        sensitive << wb_tbl_rdata;

        SC_METHOD(registers);
        sensitive << i_clk.pos();
    }

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    static const int LINES_TOTAL = 1 << abits;
    static const int WAYS_TOTAL = (1 << waybits);
    static const int LINE_WIDTH = WAYS_TOTAL * waybits;

    sc_signal<sc_uint<abits>> radr;
    sc_signal<sc_uint<abits>> wb_tbl_wadr;
    sc_uint<LINE_WIDTH> tbl[LINES_TOTAL];

    sc_signal<sc_uint<LINE_WIDTH>> wb_tbl_rdata;
    sc_signal<sc_uint<LINE_WIDTH>> wb_tbl_wdata;
    sc_signal<bool> w_we;
};


template <int abits, int waybits>
void lrunway<abits, waybits>::generateVCD(sc_trace_file *i_vcd,
                                     sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_flush, i_flush.name());
        sc_trace(o_vcd, i_addr, i_addr.name());
        sc_trace(o_vcd, i_we, i_we.name());
        sc_trace(o_vcd, i_lru, i_lru.name());
        sc_trace(o_vcd, o_lru, o_lru.name());

        std::string pn(name());
        sc_trace(o_vcd, wb_tbl_rdata, pn + ".wb_tbl_rdata");
        sc_trace(o_vcd, wb_tbl_wdata, pn + ".wb_tbl_wdata");
        sc_trace(o_vcd, wb_tbl_wadr, pn + ".wb_tbl_wadr");
        sc_trace(o_vcd, tbl[0], pn + ".tbl0");
    }
}

template <int abits, int waybits>
void lrunway<abits, waybits>::comb() {
    //sc_uint<LINE_WIDTH - waybits> vb_shift;
    sc_uint<LINE_WIDTH> vb_tbl_wdata;
    sc_uint<LINE_WIDTH> vb_tbl_rdata;
    bool v_we;
    bool shift_ena;

    vb_tbl_rdata = wb_tbl_rdata;
    wb_tbl_wadr = radr.read();

    shift_ena = 0;
    v_we = i_we.read();
    vb_tbl_wdata = vb_tbl_rdata;
    if (i_flush.read() == 1) {
        v_we = 1;
        wb_tbl_wadr = i_addr.read();
        for (int i = 0; i < WAYS_TOTAL; i++) {
            vb_tbl_wdata((i+1)*waybits-1, i*waybits) = i;
        }
        //vb_tbl_wdata = 0xe4;        // 0x3, 0x2, 0x1, 0x0
    } else if (i_we.read()) {
#if 1
        if (vb_tbl_rdata(LINE_WIDTH-1, LINE_WIDTH-waybits) != i_lru.read()) {
           vb_tbl_wdata(LINE_WIDTH-1, LINE_WIDTH-waybits) = i_lru.read();
           shift_ena = 1;

           for (int i = WAYS_TOTAL-2; i >= 0; i--) {
                if (shift_ena == 1) {
                    vb_tbl_wdata((i+1)*waybits-1, i*waybits) =
                            vb_tbl_rdata((i+2)*waybits-1, (i+1)*waybits);
                    if (vb_tbl_rdata((i+1)*waybits-1, i*waybits) == i_lru.read()) {
                        shift_ena = 0;
                    }
                } else {
                    vb_tbl_wdata((i+1)*waybits-1, i*waybits) =
                            vb_tbl_rdata((i+1)*waybits-1, i*waybits);
                }
            }
        }

#elif 1
        vb_shift = vb_tbl_rdata(LINE_WIDTH-waybits-1, 0);  // default
        if (vb_tbl_rdata(waybits-1, 0) == i_lru.read()) {
            vb_shift = vb_tbl_rdata(LINE_WIDTH-1, waybits);
        } else {
            // All except first and last elements:
            for (int i = 1; i < WAYS_TOTAL-1; i++) {
                if (vb_tbl_rdata((i+1)*waybits-1, i*waybits) == i_lru.read()) {
                    vb_shift = (vb_tbl_rdata(LINE_WIDTH-1, (i+1)*waybits),
                                vb_tbl_rdata(i*waybits-1, 0));
                }
            }
        }
        vb_tbl_wdata = (i_lru.read(), vb_shift);

#else
        sc_assert(waybits != 2); // TODO: NWAY selector

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
#endif
    }

    w_we = v_we;
    wb_tbl_wdata = vb_tbl_wdata;
    o_lru = vb_tbl_rdata(waybits-1, 0);
}

template <int abits, int waybits>
void lrunway<abits, waybits>::registers() {
    radr = i_addr.read();
    wb_tbl_rdata = tbl[i_addr.read().to_int()];
    if (w_we.read() == 1) {
        tbl[wb_tbl_wadr.read().to_int()] = wb_tbl_wdata;
    }
}

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_CACHE_MEM_LRUNWAY_H__
