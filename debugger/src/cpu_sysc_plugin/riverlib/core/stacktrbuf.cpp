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
 *
 * @brief      Stack trace buffer on hardware level.
 */

#include "stacktrbuf.h"

namespace debugger {

StackTraceBuffer::StackTraceBuffer(sc_module_name name_) : sc_module(name_) {
    SC_METHOD(comb);
    sensitive << i_raddr;
    sensitive << i_we;
    sensitive << i_waddr;
    sensitive << i_wdata;
    sensitive << raddr;

    SC_METHOD(registers);
    sensitive << i_clk.pos();
};

void StackTraceBuffer::generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd) {
}

void StackTraceBuffer::comb() {
    o_rdata = stackbuf[raddr.read()];
}

void StackTraceBuffer::registers() {
    if (i_we.read()) {
        stackbuf[i_waddr.read()] = i_wdata;
    }
    raddr = i_raddr.read();
}

}  // namespace debugger

