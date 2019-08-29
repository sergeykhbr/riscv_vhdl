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

#include "divstage64.h"

namespace debugger {

divstage64::divstage64(sc_module_name name_) : sc_module(name_),
    i_divident("i_divident"),
    i_divisor("i_divisor"),
    o_resid("o_resid"),
    o_bits("o_bits") {

    SC_METHOD(comb);
    sensitive << i_divident;
    sensitive << i_divisor;
};

void divstage64::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, o_bits, o_bits.name());
        sc_trace(o_vcd, o_resid, o_resid.name());
        sc_trace(o_vcd, i_divident, i_divident.name());
        sc_trace(o_vcd, i_divisor, i_divisor.name());

        std::string pn(name());
        sc_trace(o_vcd, wb_divx2, pn + ".wb_divx2");
        sc_trace(o_vcd, wb_divx3, pn + ".wb_divx3");
        sc_trace(o_vcd, wb_thresh[7], pn + ".wb_thresh7");
        sc_trace(o_vcd, wb_thresh[6], pn + ".wb_thresh6");
        sc_trace(o_vcd, wb_thresh[5], pn + ".wb_thresh5");
        sc_trace(o_vcd, wb_thresh[4], pn + ".wb_thresh4");
        sc_trace(o_vcd, wb_dif[3], pn + ".wb_dif3");
    }
}

void divstage64::comb() {
    sc_uint<4> wb_bits;
    sc_biguint<129> wb_divx5;
    sc_biguint<129> wb_divx7;
    sc_biguint<129> wb_divx9;
    sc_biguint<129> wb_divx11;
    sc_biguint<129> wb_divx13;
    sc_biguint<129> wb_divx15;

    wb_divident = i_divident.read().to_uint64();
    wb_divisor = i_divisor.read();
    wb_divx2 = wb_divisor << 1;
    wb_divx3 = wb_divx2 + wb_divisor;
    wb_divx5 = (wb_divisor << 2) + wb_divisor;
    wb_divx7 = (wb_divisor << 3) - wb_divisor;
    wb_divx9 = (wb_divisor << 3) + wb_divisor;
    wb_divx11 = (wb_divisor << 3) + wb_divx3;
    wb_divx13 = (wb_divisor << 4) - wb_divx3;
    wb_divx15 = (wb_divisor << 4) - wb_divisor;
#if 1
    wb_thresh[15] = (0, wb_divident) - wb_divx15;
    wb_thresh[14] = (0, wb_divident) - (wb_divx7 << 1);
    wb_thresh[13] = (0, wb_divident) - wb_divx13;
    wb_thresh[12] = (0, wb_divident) - (wb_divx3 << 2);
    wb_thresh[11] = (0, wb_divident) - wb_divx11;
    wb_thresh[10] = (0, wb_divident) - (wb_divx5 << 1);
    wb_thresh[9] = (0, wb_divident) - wb_divx9;
    wb_thresh[8] = (0, wb_divident) - (wb_divisor << 3);
    wb_thresh[7] = (0, wb_divident) - wb_divx7;
    wb_thresh[6] = (0, wb_divident) - (wb_divx3 << 1);
    wb_thresh[5] = (0, wb_divident) - wb_divx5;
    wb_thresh[4] = (0, wb_divident) - (wb_divisor << 2);
    wb_thresh[3] = (0, wb_divident) - wb_divx3;
    wb_thresh[2] = (0, wb_divident) - (wb_divisor << 1);
    wb_thresh[1] = (0, wb_divident) - wb_divisor;
    wb_thresh[0] = (0, wb_divident);

    if (wb_thresh[15][128] == 0) {
        wb_bits = 15;
        wb_dif = wb_thresh[15].range(63, 0).to_uint64();
    } else if (wb_thresh[14][128] == 0) {
        wb_bits = 14;
        wb_dif = wb_thresh[14].range(63, 0).to_uint64();
    } else if (wb_thresh[13][128] == 0) {
        wb_bits = 13;
        wb_dif = wb_thresh[13].range(63, 0).to_uint64();
    } else if (wb_thresh[12][128] == 0) {
        wb_bits = 12;
        wb_dif = wb_thresh[12].range(63, 0).to_uint64();
    } else if (wb_thresh[11][128] == 0) {
        wb_bits = 11;
        wb_dif = wb_thresh[11].range(63, 0).to_uint64();
    } else if (wb_thresh[10][128] == 0) {
        wb_bits = 10;
        wb_dif = wb_thresh[10].range(63, 0).to_uint64();
    } else if (wb_thresh[9][128] == 0) {
        wb_bits = 9;
        wb_dif = wb_thresh[9].range(63, 0).to_uint64();
    } else if (wb_thresh[8][128] == 0) {
        wb_bits = 8;
        wb_dif = wb_thresh[8].range(63, 0).to_uint64();
    } else if (wb_thresh[7][128] == 0) {
        wb_bits = 7;
        wb_dif = wb_thresh[7].range(63, 0).to_uint64();
    } else if (wb_thresh[6][128] == 0) {
        wb_bits = 6;
        wb_dif = wb_thresh[6].range(63, 0).to_uint64();
    } else if (wb_thresh[5][128] == 0) {
        wb_bits = 5;
        wb_dif = wb_thresh[5].range(63, 0).to_uint64();
    } else if (wb_thresh[4][128] == 0) {
        wb_bits = 4;
        wb_dif = wb_thresh[4].range(63, 0).to_uint64();
    } else if (wb_thresh[3][128] == 0) {
        wb_bits = 3;
        wb_dif = wb_thresh[3].range(63, 0).to_uint64();
    } else if (wb_thresh[2][128] == 0) {
        wb_bits = 2;
        wb_dif = wb_thresh[2].range(63, 0).to_uint64();
    } else if (wb_thresh[1][128] == 0) {
        wb_bits = 1;
        wb_dif = wb_thresh[1].range(63, 0).to_uint64();
    } else {
        wb_bits = 0;
        wb_dif = wb_thresh[0].range(63, 0).to_uint64();
    }

    o_bits = wb_bits;
    o_resid = wb_dif;
#else

    wb_thresh[7] = (0, wb_divident) - (wb_divx3 << 2);
    wb_thresh[6] = (0, wb_divident) - (wb_divx2 << 2);
    wb_thresh[5] = (0, wb_divident) - (wb_divisor << 2);
    wb_thresh[4] = (0, wb_divident);

    if (wb_thresh[7][128] == 0) {
        wb_bits(3, 2) = 3;
        wb_dif[2] = wb_thresh[7].range(127, 0);
    } else if (wb_thresh[6][128] == 0) {
        wb_bits(3, 2) = 2;
        wb_dif[2] = wb_thresh[6].range(127, 0);
    } else if (wb_thresh[5][128] == 0) {
        wb_bits(3, 2) = 1;
        wb_dif[2] = wb_thresh[5].range(127, 0);
    } else {
        wb_bits(3, 2) = 0;
        wb_dif[2] = wb_thresh[4].range(127, 0);
    }

    // stage 4 of 4
    wb_thresh[3] = (0, wb_dif[2]) - (wb_divx3 << 0);
    wb_thresh[2] = (0, wb_dif[2]) - (wb_divx2 << 0);
    wb_thresh[1] = (0, wb_dif[2]) - (wb_divisor << 0);
    wb_thresh[0] = (0, wb_dif[2]);

    if (wb_thresh[3][128] == 0) {
        wb_bits(1, 0) = 3;
        wb_dif[3] = wb_thresh[3].range(127, 0);
    } else if (wb_thresh[2][128] == 0) {
        wb_bits(1, 0) = 2;
        wb_dif[3] = wb_thresh[2].range(127, 0);
    } else if (wb_thresh[1][128] == 0) {
        wb_bits(1, 0) = 1;
        wb_dif[3] = wb_thresh[1].range(127, 0);
    } else {
        wb_bits(1, 0) = 0;
        wb_dif[3] = wb_thresh[0].range(127, 0);
    }
    o_bits = wb_bits;
    o_resid = wb_dif[3].range(63, 0).to_uint64();
#endif
}

}  // namespace debugger

