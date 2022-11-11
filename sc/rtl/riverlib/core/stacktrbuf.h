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
#pragma once

#include <systemc.h>
#include "../river_cfg.h"

namespace debugger {

SC_MODULE(StackTraceBuffer) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<sc_uint<CFG_LOG2_STACK_TRACE_ADDR>> i_raddr;
    sc_out<sc_biguint<(2 * RISCV_ARCH)>> o_rdata;
    sc_in<bool> i_we;
    sc_in<sc_uint<CFG_LOG2_STACK_TRACE_ADDR>> i_waddr;
    sc_in<sc_biguint<(2 * RISCV_ARCH)>> i_wdata;

    void comb();
    void registers();

    SC_HAS_PROCESS(StackTraceBuffer);

    StackTraceBuffer(sc_module_name name);


 private:
    struct StackTraceBuffer_registers {
        sc_signal<sc_uint<5>> raddr;
        sc_signal<sc_biguint<(2 * RISCV_ARCH)>> stackbuf[STACK_TRACE_BUF_SIZE];// [pc, npc]
    } v, r;


};

}  // namespace debugger

