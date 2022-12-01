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
#include "../../ambalib/types_amba.h"
#include "../river_cfg.h"
#include "../types_river.h"
#include "l2cache_lru.h"
#include "l2_amba.h"
#include "l2_dst.h"

namespace debugger {

SC_MODULE(L2Top) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_vector<sc_in<axi4_l1_out_type>> i_l1o;
    sc_vector<sc_out<axi4_l1_in_type>> o_l1i;
    sc_in<axi4_l2_in_type> i_l2i;
    sc_out<axi4_l2_out_type> o_l2o;
    sc_in<bool> i_flush_valid;


    L2Top(sc_module_name name,
          bool async_reset,
          uint32_t waybits,
          uint32_t ibits);
    virtual ~L2Top();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;
    uint32_t waybits_;
    uint32_t ibits_;

    sc_signal<bool> w_req_ready;
    sc_signal<bool> w_req_valid;
    sc_signal<sc_uint<L2_REQ_TYPE_BITS>> wb_req_type;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_req_addr;
    sc_signal<sc_uint<3>> wb_req_size;
    sc_signal<sc_uint<3>> wb_req_prot;
    sc_signal<sc_biguint<L1CACHE_LINE_BITS>> wb_req_wdata;
    sc_signal<sc_uint<L1CACHE_BYTES_PER_LINE>> wb_req_wstrb;
    sc_signal<bool> w_cache_valid;
    sc_signal<sc_biguint<L1CACHE_LINE_BITS>> wb_cache_rdata;
    sc_signal<sc_uint<2>> wb_cache_status;
    // Memory interface:
    sc_signal<bool> w_req_mem_ready;
    sc_signal<bool> w_req_mem_valid;
    sc_signal<sc_uint<REQ_MEM_TYPE_BITS>> wb_req_mem_type;
    sc_signal<sc_uint<3>> wb_req_mem_size;
    sc_signal<sc_uint<3>> wb_req_mem_prot;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_req_mem_addr;
    sc_signal<sc_uint<L2CACHE_BYTES_PER_LINE>> wb_req_mem_strob;
    sc_signal<sc_biguint<L2CACHE_LINE_BITS>> wb_req_mem_data;
    sc_signal<bool> w_mem_data_valid;
    sc_signal<bool> w_mem_data_ack;
    sc_signal<sc_biguint<L2CACHE_LINE_BITS>> wb_mem_data;
    sc_signal<bool> w_mem_load_fault;
    sc_signal<bool> w_mem_store_fault;
    // Flush interface
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_flush_address;
    sc_signal<bool> w_flush_end;

    L2CacheLru *cache0;
    L2Amba *amba0;
    L2Destination *dst0;

};

}  // namespace debugger

