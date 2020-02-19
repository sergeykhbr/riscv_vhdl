/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
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

#ifndef __DEBUGGER_RIVERLIB_CACHE_TOP_H__
#define __DEBUGGER_RIVERLIB_CACHE_TOP_H__

#include <systemc.h>
#include "../river_cfg.h"
#include "icache_lru.h"
#include "dcache_lru.h"
#include "mpu.h"
#include "../core/queue.h"

namespace debugger {

SC_MODULE(CacheTop) {
    sc_in<bool> i_clk;                                  // CPU clock
    sc_in<bool> i_nrst;                                 // Reset active LOW
    // Control path:
    sc_in<bool> i_req_ctrl_valid;                       // Control request from CPU Core is valid
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_req_ctrl_addr;     // Control request address
    sc_out<bool> o_req_ctrl_ready;                      // Control request from CPU Core is accepted
    sc_out<bool> o_resp_ctrl_valid;                     // ICache response is valid and can be accepted
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_resp_ctrl_addr;   // ICache response address
    sc_out<sc_uint<32>> o_resp_ctrl_data;               // ICache read data
    sc_out<bool> o_resp_ctrl_load_fault;                // Bus response ERRSLV or ERRDEC on read
    sc_out<bool> o_resp_ctrl_executable;                // MPU flag: executable
    sc_in<bool> i_resp_ctrl_ready;                      // CPU Core is ready to accept ICache response
    // Data path:
    sc_in<bool> i_req_data_valid;                       // Data path request from CPU Core is valid
    sc_in<bool> i_req_data_write;                       // Data write memopy operation flag
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_req_data_addr;     // Memory operation address
    sc_in<sc_uint<64>> i_req_data_wdata;                // Memory operation write value
    sc_in<sc_uint<8>> i_req_data_wstrb;                 // 8-bytes aligned strob
    sc_out<bool> o_req_data_ready;                      // Memory operation request accepted by DCache
    sc_out<bool> o_resp_data_valid;                     // DCache response is ready
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_resp_data_addr;   // DCache response address
    sc_out<sc_uint<64>> o_resp_data_data;               // DCache response read data
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_resp_data_store_fault_addr;   // AXI B-channel error
    sc_out<bool> o_resp_data_load_fault;                // Bus response ERRSLV or ERRDEC on read
    sc_out<bool> o_resp_data_store_fault;               // Bus response ERRSLV or ERRDEC on write
    sc_out<bool> o_resp_data_er_mpu_load;
    sc_out<bool> o_resp_data_er_mpu_store;
    sc_in<bool> i_resp_data_ready;                      // CPU Core is ready to accept DCache repsonse
    // Memory interface:
    sc_in<bool> i_req_mem_ready;                        // System Bus (AXI) is available
    sc_out<bool> o_req_mem_path;                        // 0=ctrl; 1=data path
    sc_out<bool> o_req_mem_valid;                       // Memory operation to system bus is valid
    sc_out<sc_uint<L1_REQ_TYPE_BITS>> o_req_mem_type;   // Memory operation type
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_req_mem_addr;     // Requesting address
    sc_out<sc_uint<L1CACHE_BYTES_PER_LINE>> o_req_mem_strob;  // Writing strob 1 bit per 1 byte (AXI compliance)
    sc_out<sc_biguint<L1CACHE_LINE_BITS>> o_req_mem_data;     // Writing value
    sc_in<bool> i_resp_mem_valid;                       // Memory operation from system bus is completed
    sc_in<bool> i_resp_mem_path;                        // 0=ctrl; 1=data path
    sc_in<sc_biguint<L1CACHE_LINE_BITS>> i_resp_mem_data;   // Read value
    sc_in<bool> i_resp_mem_load_fault;                  // Bus response with SLVERR or DECERR on read
    sc_in<bool> i_resp_mem_store_fault;                 // Bus response with SLVERR or DECERR on write
    // MPU interface
    sc_in<bool> i_mpu_region_we;
    sc_in<sc_uint<CFG_MPU_TBL_WIDTH>> i_mpu_region_idx;
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_mpu_region_addr;
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_mpu_region_mask;
    sc_in<sc_uint<CFG_MPU_FL_TOTAL>> i_mpu_region_flags;   // {ena, cachable, r, w, x}
    // D$ Snoop interface
    sc_in<bool> i_req_snoop_valid;
    sc_in<bool> i_req_snoop_getdata;                    // 0=check availability; 1=read line
    sc_out<bool> o_req_snoop_ready;
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> i_req_snoop_addr;
    sc_in<bool> i_resp_snoop_ready;
    sc_out<bool> o_resp_snoop_valid;
    sc_out<sc_biguint<L1CACHE_LINE_BITS>> o_resp_snoop_data;
    sc_out<sc_uint<DTAG_FL_TOTAL>> o_resp_snoop_flags;
    // Debug signals:
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_flush_address;     // clear ICache address from debug interface
    sc_in<bool> i_flush_valid;                          // address to clear icache is valid
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_data_flush_address;
    sc_in<bool> i_data_flush_valid;
    sc_out<bool> o_data_flush_end;
    sc_out<sc_uint<4>> o_istate;                        // ICache state machine value
    sc_out<sc_uint<4>> o_dstate;                        // DCache state machine value
    sc_out<sc_uint<2>> o_cstate;                        // cachetop state machine value

    void comb();

    SC_HAS_PROCESS(CacheTop);

    CacheTop(sc_module_name name_, bool async_reset);
    virtual ~CacheTop();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    static const int DATA_PATH = 0;
    static const int CTRL_PATH = 1;

    static const int QUEUE_WIDTH =
        CFG_CPU_ADDR_BITS       // addr
        + L1_REQ_TYPE_BITS      // req_type
        + 1                     // 0=instruction; 1=data
        ;

    struct CacheOutputType {
        sc_signal<bool> req_mem_valid;
        sc_signal<sc_uint<L1_REQ_TYPE_BITS>> req_mem_type;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_mem_addr;
        sc_signal<sc_uint<DCACHE_BYTES_PER_LINE>> req_mem_strob;
        sc_signal<sc_biguint<DCACHE_LINE_BITS>> req_mem_wdata;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> mpu_addr;
    };

    CacheOutputType i, d;

    // Memory Control interface:
    sc_signal<bool> w_ctrl_resp_mem_data_valid;
    sc_signal<sc_biguint<ICACHE_LINE_BITS>> wb_ctrl_resp_mem_data;
    sc_signal<bool> w_ctrl_resp_mem_load_fault;
    sc_signal<bool> w_resp_ctrl_writable_unused;
    sc_signal<bool> w_resp_ctrl_readable_unused;
    sc_signal<bool> w_ctrl_req_ready;
    // Memory Data interface:
    sc_signal<bool> w_data_resp_mem_data_valid;
    sc_signal<sc_biguint<DCACHE_LINE_BITS>> wb_data_resp_mem_data;
    sc_signal<bool> w_data_resp_mem_load_fault;
    sc_signal<bool> w_data_req_ready;
    sc_signal<sc_uint<CFG_MPU_FL_TOTAL>> wb_mpu_iflags;
    sc_signal<sc_uint<CFG_MPU_FL_TOTAL>> wb_mpu_dflags;

    sc_signal<bool> queue_re_i;
    sc_signal<bool> queue_we_i;
    sc_signal<sc_biguint<QUEUE_WIDTH>> queue_wdata_i;
    sc_signal<sc_biguint<QUEUE_WIDTH>> queue_rdata_o;
    sc_signal<bool> queue_full_o;
    sc_signal<bool> queue_nempty_o;

    ICacheLru *i1;
    DCacheLru *d0;
    MPU *mpu0;
    Queue<2, QUEUE_WIDTH> *queue0;
#ifdef DBG_ICACHE_TB
    ICache_tb *i0_tb;
#endif
    bool async_reset_;
};

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_CACHE_TOP_H__
