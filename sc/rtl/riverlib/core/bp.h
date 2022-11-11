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
#include "bp_btb.h"
#include "bp_predec.h"

namespace debugger {

SC_MODULE(BranchPredictor) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<bool> i_flush_pipeline;                           // sync reset BTB
    sc_in<bool> i_resp_mem_valid;                           // Memory response from ICache is valid
    sc_in<sc_uint<RISCV_ARCH>> i_resp_mem_addr;             // Memory response address
    sc_in<sc_uint<64>> i_resp_mem_data;                     // Memory response value
    sc_in<bool> i_e_jmp;                                    // jump was executed
    sc_in<sc_uint<RISCV_ARCH>> i_e_pc;                      // Previous 'Executor' instruction
    sc_in<sc_uint<RISCV_ARCH>> i_e_npc;                     // Valid instruction value awaited by 'Executor'
    sc_in<sc_uint<RISCV_ARCH>> i_ra;                        // Return address register value
    sc_out<bool> o_f_valid;                                 // Fetch request is valid
    sc_out<sc_uint<RISCV_ARCH>> o_f_pc;                     // Fetching instruction pointer
    sc_in<sc_uint<RISCV_ARCH>> i_f_requested_pc;            // already requested but not accepted address
    sc_in<sc_uint<RISCV_ARCH>> i_f_fetching_pc;             // currently memory address
    sc_in<sc_uint<RISCV_ARCH>> i_f_fetched_pc;              // already requested and fetched address
    sc_in<sc_uint<RISCV_ARCH>> i_d_pc;                      // decoded instructions

    void comb();

    SC_HAS_PROCESS(BranchPredictor);

    BranchPredictor(sc_module_name name,
                    bool async_reset);
    virtual ~BranchPredictor();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    struct PreDecType {
        sc_signal<bool> c_valid;
        sc_signal<sc_uint<RISCV_ARCH>> addr;
        sc_signal<sc_uint<32>> data;
        sc_signal<bool> jmp;
        sc_signal<sc_uint<RISCV_ARCH>> pc;
        sc_signal<sc_uint<RISCV_ARCH>> npc;
    };


    PreDecType wb_pd[2];
    sc_signal<bool> w_btb_e;
    sc_signal<bool> w_btb_we;
    sc_signal<sc_uint<RISCV_ARCH>> wb_btb_we_pc;
    sc_signal<sc_uint<RISCV_ARCH>> wb_btb_we_npc;
    sc_signal<sc_uint<RISCV_ARCH>> wb_start_pc;
    sc_signal<sc_biguint<(CFG_BP_DEPTH * RISCV_ARCH)>> wb_npc;
    sc_signal<sc_uint<CFG_BP_DEPTH>> wb_bp_exec;            // Predicted value was jump-executed before

    BpBTB *btb;
    BpPreDecoder *predec[2];

};

}  // namespace debugger

