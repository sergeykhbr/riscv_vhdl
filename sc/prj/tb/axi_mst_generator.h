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
#include "../../rtl/internal/ambalib/types_amba.h"
#include "sv_func.h"

namespace debugger {

SC_MODULE(axi_mst_generator) {
 public:
    sc_in<bool> i_nrst;
    sc_in<bool> i_clk;
    sc_in<axi4_master_in_type> i_xmst;
    sc_out<axi4_master_out_type> o_xmst;
    sc_in<bool> i_start_test;
    sc_in<sc_uint<32>> i_test_selector;
    sc_in<bool> i_show_result;
    sc_out<bool> o_writing;
    sc_out<bool> o_reading;

    void comb();
    void test();
    void registers();

    axi_mst_generator(sc_module_name name,
                      sc_uint<48> req_bar,
                      sc_uint<4> unique_id,
                      sc_uint<64> read_compare,
                      bool read_only,
                      bool burst_disable);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    sc_uint<48> req_bar_;
    sc_uint<4> unique_id_;
    sc_uint<64> read_compare_;
    bool read_only_;
    bool burst_disable_;

    struct axi_mst_generator_registers {
        sc_signal<sc_uint<32>> err_cnt;
        sc_signal<sc_uint<32>> compare_cnt;
        sc_signal<sc_uint<32>> run_cnt;
        sc_signal<sc_uint<4>> state;
        sc_signal<sc_uint<3>> xsize;
        sc_signal<sc_uint<2>> aw_wait_cnt;
        sc_signal<bool> aw_valid;
        sc_signal<sc_uint<48>> aw_addr;
        sc_signal<bool> aw_unmap;
        sc_signal<sc_uint<8>> aw_xlen;
        sc_signal<bool> w_use_axi_light;
        sc_signal<sc_uint<3>> w_wait_states;
        sc_signal<sc_uint<3>> w_wait_cnt;
        sc_signal<bool> w_valid;
        sc_signal<sc_uint<64>> w_data;
        sc_signal<sc_uint<8>> w_strb;
        sc_signal<sc_uint<8>> w_last;
        sc_signal<sc_uint<4>> w_burst_cnt;
        sc_signal<sc_uint<2>> b_wait_states;
        sc_signal<sc_uint<2>> b_wait_cnt;
        sc_signal<bool> b_ready;
        sc_signal<sc_uint<2>> ar_wait_cnt;
        sc_signal<bool> ar_valid;
        sc_signal<sc_uint<48>> ar_addr;
        sc_signal<bool> ar_unmap;
        sc_signal<sc_uint<8>> ar_xlen;
        sc_signal<sc_uint<3>> r_wait_states;
        sc_signal<sc_uint<3>> r_wait_cnt;
        sc_signal<bool> r_ready;
        sc_signal<sc_uint<4>> r_burst_cnt;
        sc_signal<bool> compare_ena;
        sc_signal<sc_uint<64>> compare_a;
        sc_signal<sc_uint<64>> compare_b;
    };

    void axi_mst_generator_r_reset(axi_mst_generator_registers& iv) {
        iv.err_cnt = 0;
        iv.compare_cnt = 0;
        iv.run_cnt = 0;
        iv.state = 0;
        iv.xsize = 3;
        iv.aw_wait_cnt = 0;
        iv.aw_valid = 0;
        iv.aw_addr = 0;
        iv.aw_unmap = 0;
        iv.aw_xlen = 0;
        iv.w_use_axi_light = 0;
        iv.w_wait_states = 0;
        iv.w_wait_cnt = 0;
        iv.w_valid = 0;
        iv.w_data = 0;
        iv.w_strb = 0;
        iv.w_last = 0;
        iv.w_burst_cnt = 0;
        iv.b_wait_states = 0;
        iv.b_wait_cnt = 0;
        iv.b_ready = 0;
        iv.ar_wait_cnt = 0;
        iv.ar_valid = 0;
        iv.ar_addr = 0;
        iv.ar_unmap = 0;
        iv.ar_xlen = 0;
        iv.r_wait_states = 0;
        iv.r_wait_cnt = 0;
        iv.r_ready = 0;
        iv.r_burst_cnt = 0;
        iv.compare_ena = 0;
        iv.compare_a = 0;
        iv.compare_b = 0;
    }

    std::string msg;
    axi_mst_generator_registers v;
    axi_mst_generator_registers r;

};

}  // namespace debugger

