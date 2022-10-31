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

SC_MODULE(ic_csr_m2_s1) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    // master[0]:
    sc_in<bool> i_m0_req_valid;
    sc_out<bool> o_m0_req_ready;
    sc_in<sc_uint<CsrReq_TotalBits>> i_m0_req_type;
    sc_in<sc_uint<12>> i_m0_req_addr;
    sc_in<sc_uint<RISCV_ARCH>> i_m0_req_data;
    sc_out<bool> o_m0_resp_valid;
    sc_in<bool> i_m0_resp_ready;
    sc_out<sc_uint<RISCV_ARCH>> o_m0_resp_data;
    sc_out<bool> o_m0_resp_exception;
    // master[1]
    sc_in<bool> i_m1_req_valid;
    sc_out<bool> o_m1_req_ready;
    sc_in<sc_uint<CsrReq_TotalBits>> i_m1_req_type;
    sc_in<sc_uint<12>> i_m1_req_addr;
    sc_in<sc_uint<RISCV_ARCH>> i_m1_req_data;
    sc_out<bool> o_m1_resp_valid;
    sc_in<bool> i_m1_resp_ready;
    sc_out<sc_uint<RISCV_ARCH>> o_m1_resp_data;
    sc_out<bool> o_m1_resp_exception;
    // slave[0]
    sc_out<bool> o_s0_req_valid;
    sc_in<bool> i_s0_req_ready;
    sc_out<sc_uint<CsrReq_TotalBits>> o_s0_req_type;
    sc_out<sc_uint<12>> o_s0_req_addr;
    sc_out<sc_uint<RISCV_ARCH>> o_s0_req_data;
    sc_in<bool> i_s0_resp_valid;
    sc_out<bool> o_s0_resp_ready;
    sc_in<sc_uint<RISCV_ARCH>> i_s0_resp_data;
    sc_in<bool> i_s0_resp_exception;

    void comb();
    void registers();

    SC_HAS_PROCESS(ic_csr_m2_s1);

    ic_csr_m2_s1(sc_module_name name,
                 bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    struct ic_csr_m2_s1_registers {
        sc_signal<bool> midx;
        sc_signal<bool> acquired;
    } v, r;

    void ic_csr_m2_s1_r_reset(ic_csr_m2_s1_registers &iv) {
        iv.midx = 0;
        iv.acquired = 0;
    }

};

}  // namespace debugger

