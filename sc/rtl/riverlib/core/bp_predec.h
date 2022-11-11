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

SC_MODULE(BpPreDecoder) {
 public:
    sc_in<bool> i_c_valid;                                  // Use compressed for prediction
    sc_in<sc_uint<RISCV_ARCH>> i_addr;                      // Memory response address
    sc_in<sc_uint<32>> i_data;                              // Memory response value
    sc_in<sc_uint<RISCV_ARCH>> i_ra;                        // Return address register value
    sc_out<bool> o_jmp;                                     // Jump detected
    sc_out<sc_uint<RISCV_ARCH>> o_pc;                       // Fetching instruction pointer
    sc_out<sc_uint<RISCV_ARCH>> o_npc;                      // Fetching instruction pointer

    void comb();

    SC_HAS_PROCESS(BpPreDecoder);

    BpPreDecoder(sc_module_name name);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    sc_signal<sc_uint<RISCV_ARCH>> vb_npc;
    sc_signal<bool> v_jal;                                  // JAL instruction
    sc_signal<bool> v_branch;                               // One of branch instructions (only negative offset)
    sc_signal<bool> v_c_j;                                  // compressed J instruction
    sc_signal<bool> v_c_ret;                                // compressed RET pseudo-instruction

};

}  // namespace debugger

