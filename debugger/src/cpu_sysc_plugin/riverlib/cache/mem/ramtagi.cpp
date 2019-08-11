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

#include "ramtagi.h"

namespace debugger {

RamTagi::RamTagi(sc_module_name name_) : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_adr;
    sensitive << i_wena;
    sensitive << i_wdata;
    sensitive << r.adr;
    sensitive << r.update;

    SC_METHOD(registers);
    sensitive << i_clk.pos();
};

void RamTagi::comb() {
    v = r;
    v.adr = i_adr.read();

    if (i_wena.read()) {
        v.mem[i_adr.read().to_int()] = i_wdata;
    }
    /** v.mem[] is not a signals, so use update register to trigger process */
    v.update = !r.update.read();

    o_rdata = r.mem[r.adr.read().to_int()];
}

void RamTagi::registers() {
    r = v;
}

}  // namespace debugger

