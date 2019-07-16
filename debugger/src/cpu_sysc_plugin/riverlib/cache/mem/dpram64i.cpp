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

#include "dpram64i.h"

namespace debugger {

DpRam64i::DpRam64i(sc_module_name name_) : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_radr;
    sensitive << i_wena;
    sensitive << i_wdata;
    sensitive << i_wadr;
    sensitive << r.radr;
    sensitive << r.update;

    SC_METHOD(registers);
    sensitive << i_clk.pos();
};

void DpRam64i::comb() {
    v = r;
    v.radr = i_radr.read();

    if (i_wena.read()) {
        v.mem[i_wadr.read()] = i_wdata;
    }
    /** v.mem[] is not a signals, so use update register to trigger process */
    v.update = !r.update.read();

    o_rdata = r.mem[r.radr.read()];
}

void DpRam64i::registers() {
    r = v;
}

}  // namespace debugger

