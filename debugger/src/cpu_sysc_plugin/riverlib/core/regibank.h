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

SC_MODULE(RegIntBank) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<sc_uint<6>> i_radr1;                              // Port 1 read address
    sc_out<sc_uint<RISCV_ARCH>> o_rdata1;                   // Port 1 read value
    sc_out<sc_uint<CFG_REG_TAG_WIDTH>> o_rtag1;             // Port 1 read tag value
    sc_in<sc_uint<6>> i_radr2;                              // Port 2 read address
    sc_out<sc_uint<RISCV_ARCH>> o_rdata2;                   // Port 2 read value
    sc_out<sc_uint<CFG_REG_TAG_WIDTH>> o_rtag2;             // Port 2 read tag value
    sc_in<sc_uint<6>> i_waddr;                              // Writing value
    sc_in<bool> i_wena;                                     // Writing is enabled
    sc_in<sc_uint<CFG_REG_TAG_WIDTH>> i_wtag;               // Writing register tag
    sc_in<sc_uint<RISCV_ARCH>> i_wdata;                     // Writing value
    sc_in<bool> i_inorder;                                  // Writing only if tag sequenced
    sc_out<bool> o_ignored;                                 // Sequenced writing is ignored because it was overwritten by executor (need for tracer)
    sc_in<sc_uint<6>> i_dport_addr;                         // Debug port address
    sc_in<bool> i_dport_ena;                                // Debug port is enabled
    sc_in<bool> i_dport_write;                              // Debug port write is enabled
    sc_in<sc_uint<RISCV_ARCH>> i_dport_wdata;               // Debug port write value
    sc_out<sc_uint<RISCV_ARCH>> o_dport_rdata;              // Debug port read value
    sc_out<sc_uint<RISCV_ARCH>> o_ra;                       // Return address for branch predictor
    sc_out<sc_uint<RISCV_ARCH>> o_sp;                       // Stack Pointer for border control
    sc_out<sc_uint<RISCV_ARCH>> o_gp;
    sc_out<sc_uint<RISCV_ARCH>> o_tp;
    sc_out<sc_uint<RISCV_ARCH>> o_t0;
    sc_out<sc_uint<RISCV_ARCH>> o_t1;
    sc_out<sc_uint<RISCV_ARCH>> o_t2;
    sc_out<sc_uint<RISCV_ARCH>> o_fp;
    sc_out<sc_uint<RISCV_ARCH>> o_s1;
    sc_out<sc_uint<RISCV_ARCH>> o_a0;
    sc_out<sc_uint<RISCV_ARCH>> o_a1;
    sc_out<sc_uint<RISCV_ARCH>> o_a2;
    sc_out<sc_uint<RISCV_ARCH>> o_a3;
    sc_out<sc_uint<RISCV_ARCH>> o_a4;
    sc_out<sc_uint<RISCV_ARCH>> o_a5;
    sc_out<sc_uint<RISCV_ARCH>> o_a6;
    sc_out<sc_uint<RISCV_ARCH>> o_a7;
    sc_out<sc_uint<RISCV_ARCH>> o_s2;
    sc_out<sc_uint<RISCV_ARCH>> o_s3;
    sc_out<sc_uint<RISCV_ARCH>> o_s4;
    sc_out<sc_uint<RISCV_ARCH>> o_s5;
    sc_out<sc_uint<RISCV_ARCH>> o_s6;
    sc_out<sc_uint<RISCV_ARCH>> o_s7;
    sc_out<sc_uint<RISCV_ARCH>> o_s8;
    sc_out<sc_uint<RISCV_ARCH>> o_s9;
    sc_out<sc_uint<RISCV_ARCH>> o_s10;
    sc_out<sc_uint<RISCV_ARCH>> o_s11;
    sc_out<sc_uint<RISCV_ARCH>> o_t3;
    sc_out<sc_uint<RISCV_ARCH>> o_t4;
    sc_out<sc_uint<RISCV_ARCH>> o_t5;
    sc_out<sc_uint<RISCV_ARCH>> o_t6;

    void comb();
    void registers();

    SC_HAS_PROCESS(RegIntBank);

    RegIntBank(sc_module_name name,
               bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    struct RegValueType {
        sc_signal<sc_uint<RISCV_ARCH>> val;
        sc_signal<sc_uint<CFG_REG_TAG_WIDTH>> tag;
    };


    struct RegIntBank_registers {
        RegValueType arr[REGS_TOTAL];
    } v, r;


};

}  // namespace debugger

