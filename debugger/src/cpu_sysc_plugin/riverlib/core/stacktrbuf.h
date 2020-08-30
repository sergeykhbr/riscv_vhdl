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
    sc_in<sc_uint<CFG_LOG2_STACK_TRACE_ADDR>> i_raddr;
    sc_out<sc_biguint<2*CFG_CPU_ADDR_BITS>> o_rdata;
    sc_in<bool> i_we;
    sc_in<sc_uint<CFG_LOG2_STACK_TRACE_ADDR>> i_waddr;
    sc_in<sc_biguint<2*CFG_CPU_ADDR_BITS>> i_wdata;

    void comb();
    void registers();

    SC_HAS_PROCESS(StackTraceBuffer);

    StackTraceBuffer(sc_module_name name_);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

private:
    sc_signal<sc_uint<5>> raddr;
    sc_signal<sc_biguint<2*CFG_CPU_ADDR_BITS>> stackbuf[STACK_TRACE_BUF_SIZE]; // [pc, npc]
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_STACKTRBUF_H__
