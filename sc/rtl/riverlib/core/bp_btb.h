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

SC_MODULE(BpBTB) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<bool> i_flush_pipeline;                           // sync reset BTB
    sc_in<bool> i_e;                                        // executed jump
    sc_in<bool> i_we;                                       // Write enable into BTB
    sc_in<sc_uint<RISCV_ARCH>> i_we_pc;                     // Jump start instruction address
    sc_in<sc_uint<RISCV_ARCH>> i_we_npc;                    // Jump target address
    sc_in<sc_uint<RISCV_ARCH>> i_bp_pc;                     // Start address of the prediction sequence
    sc_out<sc_biguint<(CFG_BP_DEPTH * RISCV_ARCH)>> o_bp_npc;// Predicted sequence
    sc_out<sc_uint<CFG_BP_DEPTH>> o_bp_exec;                // Predicted value was jump-executed before

    void comb();
    void registers();

    SC_HAS_PROCESS(BpBTB);

    BpBTB(sc_module_name name,
          bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    struct BtbEntryType {
        sc_signal<sc_uint<RISCV_ARCH>> pc;
        sc_signal<sc_uint<RISCV_ARCH>> npc;
        sc_signal<bool> exec;                               // 0=predec; 1=exec (high priority)
    };


    struct BpBTB_registers {
        BtbEntryType btb[CFG_BTB_SIZE];
    } v, r;

    sc_signal<sc_uint<RISCV_ARCH>> dbg_npc[CFG_BP_DEPTH];


};

}  // namespace debugger

