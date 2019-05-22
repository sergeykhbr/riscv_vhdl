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

#include "divstage.h"

namespace debugger {

divstage::divstage(sc_module_name name_) : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_mux_ena;
    sensitive << i_muxind;
    sensitive << i_divident;
    sensitive << i_divisor;
};

void divstage::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, o_bits, "/top/proc0/divstage0/o_bits");
    }
    sc_trace(i_vcd, i_mux_ena, "/top/proc0/dfpu0/divstage0/i_mux_ena");
    sc_trace(i_vcd, i_muxind, "/top/proc0/dfpu0/divstage0/i_muxind");
    sc_trace(i_vcd, i_divident, "/top/proc0/dfpu0/divstage0/i_divident");
    sc_trace(i_vcd, i_divisor, "/top/proc0/divstage0/i_divisor");
}

void divstage::comb() {
    sc_uint<8> wb_bits;
    sc_uint<61> wb_t15;
    sc_uint<61> wb_t11;
    sc_uint<61> wb_t7;
    sc_uint<61> wb_t3;
    sc_uint<7> wb_muxind;
    bool w_muxind_rdy;

    // stage 1 of 4
    wb_t15 = (i_divisor.read() << 7) + (i_divisor.read() << 6);
    wb_thresh[15] = i_divident.read() - wb_t15;
    wb_thresh[14] = i_divident.read() - (i_divisor.read() << 7);
    wb_thresh[13] = i_divident.read() - (i_divisor.read() << 6);
    wb_thresh[12] = (0, i_divident.read());

    if (wb_thresh[15][61] == 0) {
        wb_bits(7, 6) = 3;
        wb_dif[0] = wb_thresh[15].range(60, 0);
    } else if (wb_thresh[14][61] == 0) {
        wb_bits(7, 6) = 2;
        wb_dif[0] = wb_thresh[14].range(60, 0);
    } else if (wb_thresh[13][61] == 0) {
        wb_bits(7, 6) = 1;
        wb_dif[0] = wb_thresh[13].range(60, 0);
    } else {
        wb_bits(7, 6) = 0;
        wb_dif[0] = wb_thresh[12].range(60, 0);
    }

    // stage 2 of 4
    wb_t11 = (wb_dif[0] << 5) + (i_divisor.read() << 4);
    wb_thresh[11] = wb_dif[0] - wb_t11;
    wb_thresh[10] = wb_dif[0] - (i_divisor.read() << 5);
    wb_thresh[9] = wb_dif[0] - (i_divisor.read() << 4);
    wb_thresh[8] = (0, wb_dif[0]);

    if (wb_thresh[11][61] == 0) {
        wb_bits(5, 4) = 3;
        wb_dif[1] = wb_thresh[11].range(60, 0);
    } else if (wb_thresh[10][61] == 0) {
        wb_bits(5, 4) = 2;
        wb_dif[1] = wb_thresh[10].range(60, 0);
    } else if (wb_thresh[9][61] == 0) {
        wb_bits(5, 4) = 1;
        wb_dif[1] = wb_thresh[9].range(60, 0);
    } else {
        wb_bits(5, 4) = 0;
        wb_dif[1] = wb_thresh[8].range(60, 0);
    }

    // stage 3 of 4
    wb_t7 = (wb_dif[1] << 3) + (i_divisor.read() << 2);
    wb_thresh[7] = wb_dif[1] - wb_t7;
    wb_thresh[6] = wb_dif[1] - (i_divisor.read() << 3);
    wb_thresh[5] = wb_dif[1] - (i_divisor.read() << 2);
    wb_thresh[4] = (0, wb_dif[1]);

    if (wb_thresh[7][61] == 0) {
        wb_bits(3, 2) = 3;
        wb_dif[2] = wb_thresh[7].range(60, 0);
    } else if (wb_thresh[6][61] == 0) {
        wb_bits(3, 2) = 2;
        wb_dif[2] = wb_thresh[6].range(60, 0);
    } else if (wb_thresh[5][61] == 0) {
        wb_bits(3, 2) = 1;
        wb_dif[2] = wb_thresh[5].range(60, 0);
    } else {
        wb_bits(3, 2) = 0;
        wb_dif[2] = wb_thresh[4].range(60, 0);
    }

    // stage 4 of 4
    wb_t3 = (wb_dif[2] << 1) + i_divisor.read();
    wb_thresh[3] = wb_dif[2] - wb_t3;
    wb_thresh[2] = wb_dif[2] - (i_divisor.read() << 1);
    wb_thresh[1] = wb_dif[2] - i_divisor.read();
    wb_thresh[0] = (0, wb_dif[2]);

    if (wb_thresh[3][61] == 0) {
        wb_bits(1, 0) = 3;
        wb_dif[3] = wb_thresh[3].range(60, 0);
    } else if (wb_thresh[2][61] == 0) {
        wb_bits(1, 0) = 2;
        wb_dif[3] = wb_thresh[2].range(60, 0);
    } else if (wb_thresh[1][61] == 0) {
        wb_bits(1, 0) = 1;
        wb_dif[3] = wb_thresh[1].range(60, 0);
    } else {
        wb_bits(1, 0) = 0;
        wb_dif[3] = wb_thresh[0].range(60, 0);
    }

    // Number multiplexor
    wb_muxind = 0;
    if (i_mux_ena.read() == 1) {
        if (wb_thresh[15][61] == 0) {
            wb_muxind = i_muxind.read().range(55, 49);
        } else if (wb_thresh[14][61] == 0) {
            wb_muxind = i_muxind.read().range(55, 49);
        } else if (wb_thresh[13][61] == 0) {
            wb_muxind = i_muxind.read().range(48, 42);
        } else if (wb_thresh[11][61] == 0) {
            wb_muxind = i_muxind.read().range(41, 35);
        } else if (wb_thresh[10][61] == 0) {
            wb_muxind = i_muxind.read().range(41, 35);
        } else if (wb_thresh[9][61] == 0) {
            wb_muxind = i_muxind.read().range(34, 28);
        } else if (wb_thresh[7][61] == 0) {
            wb_muxind = i_muxind.read().range(27, 21);
        } else if (wb_thresh[6][61] == 0) {
            wb_muxind = i_muxind.read().range(27, 21);
        } else if (wb_thresh[5][61] == 0) {
            wb_muxind = i_muxind.read().range(20, 14);
        } else if (wb_thresh[3][61] == 0) {
            wb_muxind = i_muxind.read().range(13, 7);
        } else if (wb_thresh[2][61] == 0) {
            wb_muxind = i_muxind.read().range(13, 7);
        } else if (wb_thresh[1][61] == 0) {
            wb_muxind = i_muxind.read().range(6, 0);
        } else {
            wb_muxind = i_muxind.read().range(6, 0);
        }
    }

    w_muxind_rdy = 0;
    if (i_mux_ena.read() == 1 && wb_bits != 0) {
        w_muxind_rdy = 1;
    }

    o_bits = wb_bits;
    o_dif = wb_dif[3].range(52, 0);
    o_muxind = wb_muxind;
    o_muxind_rdy = w_muxind_rdy;
}

}  // namespace debugger

