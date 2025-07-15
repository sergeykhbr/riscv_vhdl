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

#include "iobuf_tech.h"
#include "api_core.h"

namespace debugger {

iobuf_tech::iobuf_tech(sc_module_name name)
    : sc_module(name),
    io("io"),
    o("o"),
    i("i"),
    t("t") {


    SC_METHOD(comb);
    sensitive << io;
    sensitive << i;
    sensitive << t;
}

void iobuf_tech::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
    if (o_vcd) {
        sc_trace(o_vcd, io, io.name());
        sc_trace(o_vcd, o, o.name());
        sc_trace(o_vcd, i, i.name());
        sc_trace(o_vcd, t, t.name());
    }

}

void iobuf_tech::comb() {
    if (t.read() == 1) {
        // IO as input:
        o = io.read();
        // io = 1'bz;
    } else {
        // IO as output:
        o = 0;
        io = i.read();
    }
}

}  // namespace debugger

