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

SC_MODULE(PMP) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<bool> i_ena;                                      // PMP is active in S or U modes or if L/MPRV bit is set in M-mode
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_iaddr;
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_daddr;
    sc_in<bool> i_we;                                       // write enable into PMP
    sc_in<sc_uint<CFG_PMP_TBL_WIDTH>> i_region;             // selected PMP region
    sc_in<sc_uint<RISCV_ARCH>> i_start_addr;                // PMP region start address
    sc_in<sc_uint<RISCV_ARCH>> i_end_addr;                  // PMP region end address (inclusive)
    sc_in<sc_uint<CFG_PMP_FL_TOTAL>> i_flags;               // {ena, lock, r, w, x}
    sc_out<bool> o_r;
    sc_out<bool> o_w;
    sc_out<bool> o_x;

    void comb();
    void registers();

    SC_HAS_PROCESS(PMP);

    PMP(sc_module_name name,
        bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    struct PmpTableItemType {
        sc_signal<sc_uint<RISCV_ARCH>> start_addr;
        sc_signal<sc_uint<RISCV_ARCH>> end_addr;
        sc_signal<sc_uint<CFG_PMP_FL_TOTAL>> flags;
    };


    struct PMP_registers {
        PmpTableItemType tbl[CFG_PMP_TBL_SIZE];
    } v, r;


};

}  // namespace debugger

