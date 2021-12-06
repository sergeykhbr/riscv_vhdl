/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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

#pragma once

#include <systemc.h>
#include "../river_cfg.h"
#include "bp_predec.h"
#include "bp_btb.h"

namespace debugger {

SC_MODULE(BranchPredictor) {
    sc_in<bool> i_clk;                  // CPU clock
    sc_in<bool> i_nrst;                 // Reset. Active LOW.
    sc_in<bool> i_flush_pipeline;       // sync reset BTB
    sc_in<bool> i_resp_mem_valid;       // Memory response from ICache is valid
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_resp_mem_addr;          // Memory response address
    sc_in<sc_uint<64>> i_resp_mem_data;                         // Memory response value
    sc_in<bool> i_e_jmp;                                        // jump was executed
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_e_pc;                   // Previous 'Executor' instruction
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_e_npc;                  // Valid instruction value awaited by 'Executor'
    sc_in<sc_uint<RISCV_ARCH>> i_ra;                            // Return address register value
    sc_out<bool> o_f_valid;                                     // Fetch request is valid
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_f_pc;                  // Fetching instruction pointer
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_f_requested_pc;         // already requested but not fetched address
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_f_fetched_pc;           // already requested and fetched address
    sc_in<sc_biguint<CFG_DEC_DEPTH*CFG_CPU_ADDR_BITS>> i_d_decoded_pc;  // decoded instructions


    void comb();

    SC_HAS_PROCESS(BranchPredictor);
    virtual ~BranchPredictor();

    BranchPredictor(sc_module_name name_, bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:

    struct PreDecType {
        sc_signal<bool> c_valid;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> addr;
        sc_signal<sc_uint<32>> data;
        sc_signal<bool> jmp;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> pc;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> npc;
    };
    PreDecType wb_pd[2];

    sc_signal<bool> w_btb_we;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_btb_we_pc;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_btb_we_npc;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_start_pc;
    sc_signal<sc_biguint<CFG_BP_DEPTH*CFG_CPU_ADDR_BITS>> wb_npc;

    BpBTB *btb;
    BpPreDecoder *predec[2];
};

}  // namespace debugger

