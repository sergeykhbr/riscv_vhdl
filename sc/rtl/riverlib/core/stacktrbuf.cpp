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

#include "stacktrbuf.h"
#include "api_core.h"

namespace debugger {

StackTraceBuffer::StackTraceBuffer(sc_module_name name)
    : sc_module(name),
    i_clk("i_clk"),
    i_raddr("i_raddr"),
    o_rdata("o_rdata"),
    i_we("i_we"),
    i_waddr("i_waddr"),
    i_wdata("i_wdata") {


    SC_METHOD(comb);
    sensitive << i_raddr;
    sensitive << i_we;
    sensitive << i_waddr;
    sensitive << i_wdata;
    sensitive << rx.raddr;
    for (int i = 0; i < STACK_TRACE_BUF_SIZE; i++) {
        sensitive << rx.stackbuf[i];
    }

    SC_METHOD(rxegisters);
    sensitive << i_clk.pos();
}

void StackTraceBuffer::comb() {
    vx.raddr = rx.raddr;
    for (int i = 0; i < STACK_TRACE_BUF_SIZE; i++) {
        vx.stackbuf[i] = rx.stackbuf[i];
    }

    vx.raddr = i_raddr;
    if (i_we.read() == 1) {
        vx.stackbuf[i_waddr.read().to_int()] = i_wdata;
    }

    o_rdata = rx.stackbuf[rx.raddr.read().to_int()];
}

void StackTraceBuffer::rxegisters() {
    rx.raddr = vx.raddr;
    for (int i = 0; i < STACK_TRACE_BUF_SIZE; i++) {
        rx.stackbuf[i] = vx.stackbuf[i];
    }
}

}  // namespace debugger

