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

    wb_divident = i_divident.read().to_uint64();
    wb_divisor = i_divisor.read();
    wb_divx2 = i_divisor.read();
    wb_divx2 <<= 1;
    wb_divx3 = wb_divx2 + i_divisor.read();

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
}

}  // namespace debugger

