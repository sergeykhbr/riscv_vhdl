/*
 *  Copyright 2020 Sergey Khabarov, sergeykhbr@gmail.com
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

#ifndef __DEBUGGER_SRC_CPU_SYSC_PLUGIN_L2_L2_TOP_H__
#define __DEBUGGER_SRC_CPU_SYSC_PLUGIN_L2_L2_TOP_H__

#include "api_core.h"
#include "../types_river.h"
#include "../../ambalib/types_amba.h"
#include "l2_dst.h"
#include "l2cache_lru.h"
#include "l2_amba.h"
#include "l2serdes.h"
#include <systemc.h>

namespace debugger {

SC_MODULE(L2Top) {
    sc_in<bool> i_clk;                                  // CPU clock
    sc_in<bool> i_nrst;                                 // Reset active LOW
    sc_in<axi4_l1_out_type> i_l1o0;
    sc_out<axi4_l1_in_type> o_l1i0;
    sc_in<axi4_l1_out_type> i_l1o1;
    sc_out<axi4_l1_in_type> o_l1i1;
    sc_in<axi4_l1_out_type> i_l1o2;
    sc_out<axi4_l1_in_type> o_l1i2;
    sc_in<axi4_l1_out_type> i_l1o3;
    sc_out<axi4_l1_in_type> o_l1i3;
    sc_in<axi4_l1_out_type> i_acpo;
    sc_out<axi4_l1_in_type> o_acpi;
    sc_in<axi4_master_in_type> i_msti;
    sc_out<axi4_master_out_type> o_msto;

    SC_HAS_PROCESS(L2Top);

    L2Top(sc_module_name name, bool async_reset);
    virtual ~L2Top();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    sc_signal<bool> w_req_ready;
    sc_signal<bool> w_req_valid;
    sc_signal<sc_uint<3>> wb_req_type;
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
    sc_signal<bool> w_req_mem_write;
    sc_signal<bool> w_req_mem_cached;
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
    sc_signal<bool> w_flush_valid;
    sc_signal<bool> w_flush_end;

    sc_signal<axi4_l2_in_type> l2i;
    sc_signal<axi4_l2_out_type> l2o;

    L2CacheLru *cache_;
    L2Amba *amba_;
    L2SerDes *serdes_;
    L2Destination *dst_;

    bool async_reset_;
};

}  // namespace debugger

#endif  // __DEBUGGER_SRC_CPU_SYSC_PLUGIN_L2_L2_TOP_H__
