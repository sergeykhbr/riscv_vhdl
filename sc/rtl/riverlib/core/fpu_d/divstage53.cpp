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

#include "divstage53.h"
#include "api_core.h"

namespace debugger {

divstage53::divstage53(sc_module_name name)
    : sc_module(name),
    i_mux_ena("i_mux_ena"),
    i_muxind("i_muxind"),
    i_divident("i_divident"),
    i_divisor("i_divisor"),
    o_dif("o_dif"),
    o_bits("o_bits"),
    o_muxind("o_muxind"),
    o_muxind_rdy("o_muxind_rdy") {


    SC_METHOD(comb);
    sensitive << i_mux_ena;
    sensitive << i_muxind;
    sensitive << i_divident;
    sensitive << i_divisor;
}

void divstage53::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, i_mux_ena, i_mux_ena.name());
        sc_trace(o_vcd, i_muxind, i_muxind.name());
        sc_trace(o_vcd, i_divident, i_divident.name());
        sc_trace(o_vcd, i_divisor, i_divisor.name());
        sc_trace(o_vcd, o_dif, o_dif.name());
        sc_trace(o_vcd, o_bits, o_bits.name());
        sc_trace(o_vcd, o_muxind, o_muxind.name());
        sc_trace(o_vcd, o_muxind_rdy, o_muxind_rdy.name());
    }

}

void divstage53::comb() {
    sc_uint<8> wb_bits;
    sc_uint<55> wb_divx3;                                   // width 53+2
    sc_uint<55> wb_divx2;                                   // width 53+2
    sc_uint<7> wb_muxind;
    bool w_muxind_rdy;

    wb_bits = 0;
    wb_divx3 = 0ull;
    wb_divx2 = 0ull;
    wb_muxind = 0;
    w_muxind_rdy = 0;

    wb_divx2 = (i_divisor.read() << 1);
    wb_divx3 = (wb_divx2 + i_divisor.read());

    // stage 1 of 4
    wb_thresh[15] = ((0, i_divident.read()) - (wb_divx3.to_uint64() << 6));
    wb_thresh[14] = ((0, i_divident.read()) - (wb_divx2.to_uint64() << 6));
    wb_thresh[13] = ((0, i_divident.read()) - (i_divisor.read().to_uint64() << 6));
    wb_thresh[12] = (0, i_divident.read());

    if (wb_thresh[15][61] == 0) {
        wb_bits(7, 6) = 3;
        wb_dif[0] = wb_thresh[15](60, 0);
    } else if (wb_thresh[14][61] == 0) {
        wb_bits(7, 6) = 2;
        wb_dif[0] = wb_thresh[14](60, 0);
    } else if (wb_thresh[13][61] == 0) {
        wb_bits(7, 6) = 1;
        wb_dif[0] = wb_thresh[13](60, 0);
    } else {
        wb_bits(7, 6) = 0;
        wb_dif[0] = wb_thresh[12](60, 0);
    }

    // stage 2 of 4
    wb_thresh[11] = ((0, wb_dif[0]) - (wb_divx3.to_uint64() << 4));
    wb_thresh[10] = ((0, wb_dif[0]) - (wb_divx2.to_uint64() << 4));
    wb_thresh[9] = ((0, wb_dif[0]) - (i_divisor.read().to_uint64() << 4));
    wb_thresh[8] = (0, wb_dif[0]);

    if (wb_thresh[11][61] == 0) {
        wb_bits(5, 4) = 3;
        wb_dif[1] = wb_thresh[11](60, 0);
    } else if (wb_thresh[10][61] == 0) {
        wb_bits(5, 4) = 2;
        wb_dif[1] = wb_thresh[10](60, 0);
    } else if (wb_thresh[9][61] == 0) {
        wb_bits(5, 4) = 1;
        wb_dif[1] = wb_thresh[9](60, 0);
    } else {
        wb_bits(5, 4) = 0;
        wb_dif[1] = wb_thresh[8](60, 0);
    }

    // stage 3 of 4
    wb_thresh[7] = ((0, wb_dif[1]) - (wb_divx3.to_uint64() << 2));
    wb_thresh[6] = ((0, wb_dif[1]) - (wb_divx2.to_uint64() << 2));
    wb_thresh[5] = ((0, wb_dif[1]) - (i_divisor.read().to_uint64() << 2));
    wb_thresh[4] = (0, wb_dif[1]);
    if (wb_thresh[7][61] == 0) {
        wb_bits(3, 2) = 3;
        wb_dif[2] = wb_thresh[7](60, 0);
    } else if (wb_thresh[6][61] == 0) {
        wb_bits(3, 2) = 2;
        wb_dif[2] = wb_thresh[6](60, 0);
    } else if (wb_thresh[5][61] == 0) {
        wb_bits(3, 2) = 1;
        wb_dif[2] = wb_thresh[5](60, 0);
    } else {
        wb_bits(3, 2) = 0;
        wb_dif[2] = wb_thresh[4](60, 0);
    }

    // stage 4 of 4
    wb_thresh[3] = ((0, wb_dif[2]) - (0, wb_divx3.to_uint64()));
    wb_thresh[2] = ((0, wb_dif[2]) - (0, wb_divx2.to_uint64()));
    wb_thresh[1] = ((0, wb_dif[2]) - (0, i_divisor.read().to_uint64()));
    wb_thresh[0] = (0, wb_dif[2]);
    if (wb_thresh[3][61] == 0) {
        wb_bits(1, 0) = 3;
        wb_dif[3] = wb_thresh[3](60, 0);
    } else if (wb_thresh[2][61] == 0) {
        wb_bits(1, 0) = 2;
        wb_dif[3] = wb_thresh[2](60, 0);
    } else if (wb_thresh[1][61] == 0) {
        wb_bits(1, 0) = 1;
        wb_dif[3] = wb_thresh[1](60, 0);
    } else {
        wb_bits(1, 0) = 0;
        wb_dif[3] = wb_thresh[0](60, 0);
    }

    // Number multiplexor
    wb_muxind = 0;
    if (i_mux_ena.read() == 1) {
        if (wb_thresh[15][61] == 0) {
            wb_muxind = i_muxind.read()(55, 49);
        } else if (wb_thresh[14][61] == 0) {
            wb_muxind = i_muxind.read()(55, 49);
        } else if (wb_thresh[13][61] == 0) {
            wb_muxind = i_muxind.read()(48, 42);
        } else if (wb_thresh[11][61] == 0) {
            wb_muxind = i_muxind.read()(41, 35);
        } else if (wb_thresh[10][61] == 0) {
            wb_muxind = i_muxind.read()(41, 35);
        } else if (wb_thresh[9][61] == 0) {
            wb_muxind = i_muxind.read()(34, 28);
        } else if (wb_thresh[7][61] == 0) {
            wb_muxind = i_muxind.read()(27, 21);
        } else if (wb_thresh[6][61] == 0) {
            wb_muxind = i_muxind.read()(27, 21);
        } else if (wb_thresh[5][61] == 0) {
            wb_muxind = i_muxind.read()(20, 14);
        } else if (wb_thresh[3][61] == 0) {
            wb_muxind = i_muxind.read()(13, 7);
        } else if (wb_thresh[2][61] == 0) {
            wb_muxind = i_muxind.read()(13, 7);
        } else if (wb_thresh[1][61] == 0) {
            wb_muxind = i_muxind.read()(6, 0);
        } else {
            wb_muxind = i_muxind.read()(6, 0);
        }
    }

    w_muxind_rdy = 0;
    if ((i_mux_ena.read() == 1) && (wb_bits.or_reduce() == 1)) {
        w_muxind_rdy = 1;
    }

    o_bits = wb_bits;
    o_dif = wb_dif[3](52, 0);
    o_muxind = wb_muxind;
    o_muxind_rdy = w_muxind_rdy;
}

}  // namespace debugger

