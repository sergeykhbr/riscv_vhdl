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
    sensitive << r.adr;
    sensitive << r.update;

    SC_METHOD(registers);
    sensitive << i_clk.pos();
};

void ILru::comb() {
    sc_uint<8> wb_lru;

    v = r;
    v.adr = i_adr.read();

    wb_lru = r.tbl[r.adr.read()];


    if (i_init.read() == 1) {
        v.tbl[i_adr.read()] = 0xe4;        // 0x3, 0x2, 0x1, 0x0
    } else if (i_we.read() && wb_lru(7, 6) != i_lru.read()) {
        v.tbl[i_adr.read()] = (i_lru.read(), wb_lru(7, 2));
    }
    /** v.mem[] is not a signals, so use update register to trigger process */
    v.update = !r.update.read();

    o_lru = wb_lru(1, 0);
}

void ILru::registers() {
    r = v;
}

}  // namespace debugger

