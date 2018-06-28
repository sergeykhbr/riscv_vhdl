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

#ifndef __DEBUGGER_RIVERLIB_BR_PREDIC_H__
#define __DEBUGGER_RIVERLIB_BR_PREDIC_H__

#include <systemc.h>
#include "../river_cfg.h"

namespace debugger {

SC_MODULE(BranchPredictor) {
    sc_in<bool> i_clk;                  // CPU clock
    sc_in<bool> i_nrst;                 // Reset. Active LOW.
    sc_in<bool> i_req_mem_fire;         // Memory request was accepted
    sc_in<bool> i_resp_mem_valid;       // Memory response from ICache is valid
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_resp_mem_addr; // Memory response address
    sc_in<sc_uint<32>> i_resp_mem_data; // Memory response value
    sc_in<bool> i_f_predic_miss;        // Fetch modul detects deviation between predicted and valid pc.
    sc_in<sc_uint<32>> i_e_npc;         // Valid instruction value awaited by 'Executor'
    sc_in<sc_uint<RISCV_ARCH>> i_ra;    // Return address register value
    sc_out<sc_uint<32>> o_npc_predict;  // Predicted next instruction address

    void comb();
    void registers();

    SC_HAS_PROCESS(BranchPredictor);

    BranchPredictor(sc_module_name name_);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

private:
    struct RegistersType {
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> npc;
        sc_signal<bool> predicted;
        sc_signal<bool> compressed;
    } v, r;
    sc_uint<BUS_ADDR_WIDTH> wb_npc;
    bool w_compressed;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_BR_PREDIC_H__
