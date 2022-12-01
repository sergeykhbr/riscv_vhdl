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
#include "tagmemcoupled.h"

namespace debugger {

SC_MODULE(ICacheLru) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    // Control path:
    sc_in<bool> i_req_valid;
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_req_addr;
    sc_out<bool> o_req_ready;
    sc_out<bool> o_resp_valid;
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_resp_addr;
    sc_out<sc_uint<64>> o_resp_data;
    sc_out<bool> o_resp_load_fault;
    sc_in<bool> i_resp_ready;
    // Memory interface:
    sc_in<bool> i_req_mem_ready;
    sc_out<bool> o_req_mem_valid;
    sc_out<sc_uint<REQ_MEM_TYPE_BITS>> o_req_mem_type;
    sc_out<sc_uint<3>> o_req_mem_size;
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_req_mem_addr;
    sc_out<sc_uint<L1CACHE_BYTES_PER_LINE>> o_req_mem_strob;// unused
    sc_out<sc_biguint<L1CACHE_LINE_BITS>> o_req_mem_data;
    sc_in<bool> i_mem_data_valid;
    sc_in<sc_biguint<L1CACHE_LINE_BITS>> i_mem_data;
    sc_in<bool> i_mem_load_fault;
    // Mpu interface
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_mpu_addr;
    sc_in<bool> i_pma_cached;
    sc_in<bool> i_pmp_x;                                    // PMP eXecute access
    // Flush interface
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_flush_address;
    sc_in<bool> i_flush_valid;

    void comb();
    void registers();

    SC_HAS_PROCESS(ICacheLru);

    ICacheLru(sc_module_name name,
              bool async_reset,
              uint32_t waybits,
              uint32_t ibits);
    virtual ~ICacheLru();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;
    uint32_t waybits_;
    uint32_t ibits_;
    int ways;
    uint32_t FLUSH_ALL_VALUE;

    static const int abus = CFG_CPU_ADDR_BITS;
    static const int lnbits = CFG_LOG2_L1CACHE_BYTES_PER_LINE;
    static const int flbits = ITAG_FL_TOTAL;
    // State machine states:
    static const uint8_t State_Idle = 0;
    static const uint8_t State_CheckHit = 1;
    static const uint8_t State_TranslateAddress = 2;
    static const uint8_t State_WaitGrant = 3;
    static const uint8_t State_WaitResp = 4;
    static const uint8_t State_CheckResp = 5;
    static const uint8_t State_SetupReadAdr = 6;
    static const uint8_t State_FlushAddr = 8;
    static const uint8_t State_FlushCheck = 9;
    static const uint8_t State_Reset = 10;
    static const uint8_t State_ResetWrite = 11;
    
    static const uint64_t LINE_BYTES_MASK = ((1 << CFG_LOG2_L1CACHE_BYTES_PER_LINE) - 1);

    struct ICacheLru_registers {
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_addr;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_addr_next;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> write_addr;
        sc_signal<sc_uint<4>> state;
        sc_signal<bool> req_mem_valid;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> mem_addr;
        sc_signal<sc_uint<REQ_MEM_TYPE_BITS>> req_mem_type;
        sc_signal<sc_uint<3>> req_mem_size;
        sc_signal<bool> load_fault;
        sc_signal<bool> req_flush;                          // init flush request
        sc_signal<bool> req_flush_all;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_flush_addr;// [0]=1 flush all
        sc_signal<sc_uint<32>> req_flush_cnt;
        sc_signal<sc_uint<32>> flush_cnt;
        sc_signal<sc_biguint<L1CACHE_LINE_BITS>> cache_line_i;
    } v, r;

    void ICacheLru_r_reset(ICacheLru_registers &iv) {
        iv.req_addr = 0ull;
        iv.req_addr_next = 0ull;
        iv.write_addr = 0ull;
        iv.state = State_Reset;
        iv.req_mem_valid = 0;
        iv.mem_addr = 0ull;
        iv.req_mem_type = 0;
        iv.req_mem_size = 0;
        iv.load_fault = 0;
        iv.req_flush = 1;
        iv.req_flush_all = 0;
        iv.req_flush_addr = 0ull;
        iv.req_flush_cnt = 0;
        iv.flush_cnt = 0;
        iv.cache_line_i = 0ull;
    }

    sc_signal<bool> line_direct_access_i;
    sc_signal<bool> line_invalidate_i;
    sc_signal<bool> line_re_i;
    sc_signal<bool> line_we_i;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> line_addr_i;
    sc_signal<sc_biguint<L1CACHE_LINE_BITS>> line_wdata_i;
    sc_signal<sc_uint<(1 << CFG_LOG2_L1CACHE_BYTES_PER_LINE)>> line_wstrb_i;
    sc_signal<sc_uint<ITAG_FL_TOTAL>> line_wflags_i;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> line_raddr_o;
    sc_signal<sc_biguint<(L1CACHE_LINE_BITS + 32)>> line_rdata_o;
    sc_signal<sc_uint<ITAG_FL_TOTAL>> line_rflags_o;
    sc_signal<bool> line_hit_o;
    sc_signal<bool> line_hit_next_o;

    TagMemCoupled<abus, 2, 7, lnbits, flbits> *mem0;

};

}  // namespace debugger

