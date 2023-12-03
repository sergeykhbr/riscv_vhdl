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
#include "sdctrl_cfg.h"
#include "../riverlib/cache/tagmem.h"

namespace debugger {

SC_MODULE(sdctrl_cache) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    // Data path:
    sc_in<bool> i_req_valid;
    sc_in<bool> i_req_write;
    sc_in<sc_uint<CFG_SDCACHE_ADDR_BITS>> i_req_addr;
    sc_in<sc_uint<64>> i_req_wdata;
    sc_in<sc_uint<8>> i_req_wstrb;
    sc_out<bool> o_req_ready;
    sc_out<bool> o_resp_valid;
    sc_out<sc_uint<64>> o_resp_data;
    sc_out<bool> o_resp_err;
    sc_in<bool> i_resp_ready;
    // Memory interface:
    sc_in<bool> i_req_mem_ready;
    sc_out<bool> o_req_mem_valid;
    sc_out<bool> o_req_mem_write;
    sc_out<sc_uint<CFG_SDCACHE_ADDR_BITS>> o_req_mem_addr;
    sc_out<sc_biguint<SDCACHE_LINE_BITS>> o_req_mem_data;
    sc_in<bool> i_mem_data_valid;
    sc_in<sc_biguint<SDCACHE_LINE_BITS>> i_mem_data;
    sc_in<bool> i_mem_fault;
    // Debug interface
    sc_in<bool> i_flush_valid;
    sc_out<bool> o_flush_end;

    void comb();
    void registers();

    SC_HAS_PROCESS(sdctrl_cache);

    sdctrl_cache(sc_module_name name,
                 bool async_reset);
    virtual ~sdctrl_cache();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    static const int abus = CFG_SDCACHE_ADDR_BITS;
    static const int ibits = CFG_LOG2_SDCACHE_LINEBITS;
    static const int lnbits = CFG_LOG2_SDCACHE_BYTES_PER_LINE;
    static const int flbits = SDCACHE_FL_TOTAL;

    // State machine states:
    static const uint8_t State_Idle = 0;
    static const uint8_t State_CheckHit = 2;
    static const uint8_t State_TranslateAddress = 3;
    static const uint8_t State_WaitGrant = 4;
    static const uint8_t State_WaitResp = 5;
    static const uint8_t State_CheckResp = 6;
    static const uint8_t State_SetupReadAdr = 1;
    static const uint8_t State_WriteBus = 7;
    static const uint8_t State_FlushAddr = 8;
    static const uint8_t State_FlushCheck = 9;
    static const uint8_t State_Reset = 10;
    static const uint8_t State_ResetWrite = 11;

    static const uint64_t LINE_BYTES_MASK = ((1 << CFG_LOG2_SDCACHE_BYTES_PER_LINE) - 1);
    static const uint32_t FLUSH_ALL_VALUE = ((1 << ibits) - 1);

    struct sdctrl_cache_registers {
        sc_signal<bool> req_write;
        sc_signal<sc_uint<CFG_SDCACHE_ADDR_BITS>> req_addr;
        sc_signal<sc_uint<64>> req_wdata;
        sc_signal<sc_uint<8>> req_wstrb;
        sc_signal<sc_uint<4>> state;
        sc_signal<bool> req_mem_valid;
        sc_signal<bool> req_mem_write;
        sc_signal<sc_uint<CFG_SDCACHE_ADDR_BITS>> mem_addr;
        sc_signal<bool> mem_fault;
        sc_signal<bool> write_first;
        sc_signal<bool> write_flush;
        sc_signal<bool> req_flush;
        sc_signal<sc_uint<32>> flush_cnt;
        sc_signal<sc_uint<CFG_SDCACHE_ADDR_BITS>> line_addr_i;
        sc_signal<sc_biguint<SDCACHE_LINE_BITS>> cache_line_i;
        sc_signal<sc_biguint<SDCACHE_LINE_BITS>> cache_line_o;
    } v, r;

    void sdctrl_cache_r_reset(sdctrl_cache_registers &iv) {
        iv.req_write = 0;
        iv.req_addr = 0;
        iv.req_wdata = 0;
        iv.req_wstrb = 0;
        iv.state = State_Reset;
        iv.req_mem_valid = 0;
        iv.req_mem_write = 0;
        iv.mem_addr = 0;
        iv.mem_fault = 0;
        iv.write_first = 0;
        iv.write_flush = 0;
        iv.req_flush = 0;
        iv.flush_cnt = 0;
        iv.line_addr_i = 0;
        iv.cache_line_i = 0;
        iv.cache_line_o = 0;
    }

    sc_signal<sc_biguint<SDCACHE_LINE_BITS>> line_wdata_i;
    sc_signal<sc_uint<SDCACHE_BYTES_PER_LINE>> line_wstrb_i;
    sc_signal<sc_uint<SDCACHE_FL_TOTAL>> line_wflags_i;
    sc_signal<sc_uint<CFG_SDCACHE_ADDR_BITS>> line_raddr_o;
    sc_signal<sc_biguint<SDCACHE_LINE_BITS>> line_rdata_o;
    sc_signal<sc_uint<SDCACHE_FL_TOTAL>> line_rflags_o;
    sc_signal<bool> line_hit_o;
    // Snoop signals:
    sc_signal<sc_uint<CFG_SDCACHE_ADDR_BITS>> line_snoop_addr_i;
    sc_signal<sc_uint<SDCACHE_FL_TOTAL>> line_snoop_flags_o;

    TagMem<abus, ibits, lnbits, flbits, 0> *mem0;

};

}  // namespace debugger

