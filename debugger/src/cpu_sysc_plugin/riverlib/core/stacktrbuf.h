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

#ifndef __DEBUGGER_RIVERLIB_STACKTRBUF_H__
#define __DEBUGGER_RIVERLIB_STACKTRBUF_H__

#include <systemc.h>
#include "../river_cfg.h"

namespace debugger {

SC_MODULE(StackTraceBuffer) {
    sc_in<bool> i_clk;
    sc_in<sc_uint<5>> i_raddr;                   // todo: log2(CFG_STACK_TRACE_BUF_SIZE)
    sc_out<sc_biguint<2*BUS_ADDR_WIDTH>> o_rdata;
    sc_in<bool> i_we;
    sc_in<sc_uint<5>> i_waddr;                   // todo: log2(CFG_STACK_TRACE_BUF_SIZE)
    sc_in<sc_biguint<2*BUS_ADDR_WIDTH>> i_wdata;

    void comb();
    void registers();

    SC_HAS_PROCESS(StackTraceBuffer);

    StackTraceBuffer(sc_module_name name_);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

private:
    sc_signal<sc_uint<5>> raddr;
    sc_signal<sc_biguint<2*BUS_ADDR_WIDTH>> stackbuf[CFG_STACK_TRACE_BUF_SIZE]; // [pc, npc]
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_STACKTRBUF_H__
