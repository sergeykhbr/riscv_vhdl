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
#include "icache_lru.h"
#include "dcache_lru.h"
#include "pma.h"
#include "pmp.h"
#include "../core/queue.h"

namespace debugger {

SC_MODULE(CacheTop) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    // Control path:
    sc_in<bool> i_req_ctrl_valid;                           // Control request from CPU Core is valid
    sc_in<sc_uint<RISCV_ARCH>> i_req_ctrl_addr;             // Control request address
    sc_out<bool> o_req_ctrl_ready;                          // Control request from CPU Core is accepted
    sc_out<bool> o_resp_ctrl_valid;                         // ICache response is valid and can be accepted
    sc_out<sc_uint<RISCV_ARCH>> o_resp_ctrl_addr;           // ICache response address
    sc_out<sc_uint<64>> o_resp_ctrl_data;                   // ICache read data
    sc_out<bool> o_resp_ctrl_load_fault;                    // Bus response ERRSLV or ERRDEC on read
    sc_in<bool> i_resp_ctrl_ready;                          // CPU Core is ready to accept ICache response
    // Data path:
    sc_in<bool> i_req_data_valid;                           // Data path request from CPU Core is valid
    sc_in<sc_uint<MemopType_Total>> i_req_data_type;        // Data write memopy operation flag
    sc_in<sc_uint<RISCV_ARCH>> i_req_data_addr;             // Memory operation address
    sc_in<sc_uint<64>> i_req_data_wdata;                    // Memory operation write value
    sc_in<sc_uint<8>> i_req_data_wstrb;                     // 8-bytes aligned strob
    sc_in<sc_uint<2>> i_req_data_size;
    sc_out<bool> o_req_data_ready;                          // Memory operation request accepted by DCache
    sc_out<bool> o_resp_data_valid;                         // DCache response is ready
    sc_out<sc_uint<RISCV_ARCH>> o_resp_data_addr;           // DCache response address
    sc_out<sc_uint<64>> o_resp_data_data;                   // DCache response read data
    sc_out<bool> o_resp_data_load_fault;                    // Bus response ERRSLV or ERRDEC on read
    sc_out<bool> o_resp_data_store_fault;                   // Bus response ERRSLV or ERRDEC on write
    sc_in<bool> i_resp_data_ready;                          // CPU Core is ready to accept DCache repsonse
    // Memory interface:
    sc_in<bool> i_req_mem_ready;                            // System Bus is ready to accept memory operation request
    sc_out<bool> o_req_mem_path;                            // 1=ctrl; 0=data path
    sc_out<bool> o_req_mem_valid;                           // AXI memory request is valid
    sc_out<sc_uint<REQ_MEM_TYPE_BITS>> o_req_mem_type;      // AXI memory request type
    sc_out<sc_uint<3>> o_req_mem_size;                      // request size: 0=1 B;...; 7=128 B
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_req_mem_addr;      // AXI memory request address
    sc_out<sc_uint<L1CACHE_BYTES_PER_LINE>> o_req_mem_strob;// Writing strob. 1 bit per Byte (uncached only)
    sc_out<sc_biguint<L1CACHE_LINE_BITS>> o_req_mem_data;   // Writing data
    sc_in<bool> i_resp_mem_valid;                           // AXI response is valid
    sc_in<bool> i_resp_mem_path;                            // 0=ctrl; 1=data path
    sc_in<sc_biguint<L1CACHE_LINE_BITS>> i_resp_mem_data;   // Read data
    sc_in<bool> i_resp_mem_load_fault;                      // data load error
    sc_in<bool> i_resp_mem_store_fault;                     // data store error
    // PMP interface:
    sc_in<bool> i_pmp_ena;                                  // PMP is active in S or U modes or if L/MPRV bit is set in M-mode
    sc_in<bool> i_pmp_we;                                   // write enable into PMP
    sc_in<sc_uint<CFG_PMP_TBL_WIDTH>> i_pmp_region;         // selected PMP region
    sc_in<sc_uint<RISCV_ARCH>> i_pmp_start_addr;            // PMP region start address
    sc_in<sc_uint<RISCV_ARCH>> i_pmp_end_addr;              // PMP region end address (inclusive)
    sc_in<sc_uint<CFG_PMP_FL_TOTAL>> i_pmp_flags;           // {ena, lock, r, w, x}
    // $D Snoop interface:
    sc_in<bool> i_req_snoop_valid;
    sc_in<sc_uint<SNOOP_REQ_TYPE_BITS>> i_req_snoop_type;
    sc_out<bool> o_req_snoop_ready;
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_req_snoop_addr;
    sc_in<bool> i_resp_snoop_ready;
    sc_out<bool> o_resp_snoop_valid;
    sc_out<sc_biguint<L1CACHE_LINE_BITS>> o_resp_snoop_data;
    sc_out<sc_uint<DTAG_FL_TOTAL>> o_resp_snoop_flags;
    // Debug signals:
    sc_in<bool> i_flushi_valid;                             // address to clear icache is valid
    sc_in<sc_uint<RISCV_ARCH>> i_flushi_addr;               // clear ICache address from debug interface
    sc_in<bool> i_flushd_valid;
    sc_in<sc_uint<RISCV_ARCH>> i_flushd_addr;
    sc_out<bool> o_flushd_end;

    void comb();

    SC_HAS_PROCESS(CacheTop);

    CacheTop(sc_module_name name,
             bool async_reset,
             bool coherence_ena,
             uint32_t ilog2_nways,
             uint32_t ilog2_lines_per_way,
             uint32_t dlog2_nways,
             uint32_t dlog2_lines_per_way);
    virtual ~CacheTop();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;
    bool coherence_ena_;
    uint32_t ilog2_nways_;
    uint32_t ilog2_lines_per_way_;
    uint32_t dlog2_nways_;
    uint32_t dlog2_lines_per_way_;

    static const int DATA_PATH = 0;
    static const int CTRL_PATH = 1;
    static const int QUEUE_WIDTH = (CFG_CPU_ADDR_BITS  // o_req_mem_addr
            + REQ_MEM_TYPE_BITS  // o_req_mem_type
            + 3  // o_req_mem_size
            + 1  // i_resp_mem_path
    );

    struct CacheOutputType {
        sc_signal<bool> req_mem_valid;
        sc_signal<sc_uint<REQ_MEM_TYPE_BITS>> req_mem_type;
        sc_signal<sc_uint<3>> req_mem_size;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_mem_addr;
        sc_signal<sc_uint<L1CACHE_BYTES_PER_LINE>> req_mem_strob;
        sc_signal<sc_biguint<L1CACHE_LINE_BITS>> req_mem_wdata;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> mpu_addr;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> resp_addr;
    };


    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_i_req_ctrl_addr;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_i_req_data_addr;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_i_flushi_addr;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_i_flushd_addr;
    CacheOutputType i;
    CacheOutputType d;
    // Memory Control interface:
    sc_signal<bool> w_ctrl_resp_mem_data_valid;
    sc_signal<sc_biguint<L1CACHE_LINE_BITS>> wb_ctrl_resp_mem_data;
    sc_signal<bool> w_ctrl_resp_mem_load_fault;
    sc_signal<bool> w_ctrl_req_ready;
    // Memory Data interface:
    sc_signal<bool> w_data_resp_mem_data_valid;
    sc_signal<sc_biguint<L1CACHE_LINE_BITS>> wb_data_resp_mem_data;
    sc_signal<bool> w_data_resp_mem_load_fault;
    sc_signal<bool> w_data_req_ready;
    sc_signal<bool> w_pma_icached;
    sc_signal<bool> w_pma_dcached;
    sc_signal<bool> w_pmp_r;
    sc_signal<bool> w_pmp_w;
    sc_signal<bool> w_pmp_x;
    // Queue interface
    sc_signal<bool> queue_re_i;
    sc_signal<bool> queue_we_i;
    sc_signal<sc_biguint<QUEUE_WIDTH>> queue_wdata_i;
    sc_signal<sc_biguint<QUEUE_WIDTH>> queue_rdata_o;
    sc_signal<bool> queue_full_o;
    sc_signal<bool> queue_nempty_o;

    ICacheLru *i1;
    DCacheLru *d0;
    PMA *pma0;
    PMP *pmp0;
    Queue<2, QUEUE_WIDTH> *queue0;

};

}  // namespace debugger

