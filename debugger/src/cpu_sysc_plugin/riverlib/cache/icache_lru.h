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

#ifndef __DEBUGGER_RIVERLIB_ICACHE_LRU_H__
#define __DEBUGGER_RIVERLIB_ICACHE_LRU_H__

#include <systemc.h>
#include "../river_cfg.h"
#include "tagmemcoupled.h"

namespace debugger {

//#define DBG_ICACHE_LRU_TB

SC_MODULE(ICacheLru) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    // Control path:
    sc_in<bool> i_req_valid;
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_req_addr;
    sc_out<bool> o_req_ready;
    sc_out<bool> o_resp_valid;
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_resp_addr;
    sc_out<sc_uint<32>> o_resp_data;
    sc_out<bool> o_resp_load_fault;
    sc_out<bool> o_resp_executable;
    sc_out<bool> o_resp_writable;
    sc_out<bool> o_resp_readable;
    sc_in<bool> i_resp_ready;
    // Memory interface:
    sc_in<bool> i_req_mem_ready;
    sc_out<bool> o_req_mem_valid;
    sc_out<sc_uint<REQ_MEM_TYPE_BITS>> o_req_mem_type;
    sc_out<sc_uint<3>> o_req_mem_size;
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_req_mem_addr;
    sc_out<sc_uint<ICACHE_BYTES_PER_LINE>> o_req_mem_strob;    // unused
    sc_out<sc_biguint<ICACHE_LINE_BITS>> o_req_mem_data;
    sc_in<bool> i_mem_data_valid;
    sc_in<sc_biguint<ICACHE_LINE_BITS>> i_mem_data;
    sc_in<bool> i_mem_load_fault;
    // Mpu interface
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_mpu_addr;
    sc_in<sc_uint<CFG_MPU_FL_TOTAL>> i_mpu_flags;
    // Debug interface
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_flush_address;
    sc_in<bool> i_flush_valid;

    void comb();
    void registers();

    SC_HAS_PROCESS(ICacheLru);

    ICacheLru(sc_module_name name_, bool async_reset);
    virtual ~ICacheLru();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    enum EState {
        State_Idle,
        State_CheckHit,
        State_TranslateAddress,
        State_WaitGrant,
        State_WaitResp,
        State_CheckResp,
        State_SetupReadAdr,
        State_FlushAddr,
        State_FlushCheck,
        State_Reset,
        State_ResetWrite,
    };

    struct RegistersType {
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_addr;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_addr_next;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> write_addr;
        sc_signal<sc_uint<4>> state;
        sc_signal<bool> req_mem_valid;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> mem_addr;
        sc_signal<sc_uint<REQ_MEM_TYPE_BITS>> req_mem_type;
        sc_signal<sc_uint<3>> req_mem_size;
        sc_signal<bool> executable;
        sc_signal<bool> load_fault;
        sc_signal<bool> req_flush;
        sc_signal<bool> req_flush_all;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_flush_addr;
        sc_signal<sc_uint<CFG_ILOG2_LINES_PER_WAY+CFG_ILOG2_NWAYS>> req_flush_cnt;
        sc_signal<sc_uint<CFG_ILOG2_LINES_PER_WAY+CFG_ILOG2_NWAYS>> flush_cnt;
        sc_signal<sc_biguint<ICACHE_LINE_BITS>> cache_line_i;
    } v, r;

    void R_RESET(RegistersType &iv) {
        iv.req_addr = 0;
        iv.req_addr_next = 0;
        iv.write_addr = 0;
        iv.state = State_Reset;
        iv.req_mem_valid = 0;
        iv.mem_addr = 0;
        iv.req_mem_type = 0;
        iv.req_mem_size = 0;
        iv.executable = 0;
        iv.load_fault = 0;
        iv.req_flush = 0;           // init flush request
        iv.req_flush_all = 0;
        iv.req_flush_addr = 0;   // [0]=1 flush all
        iv.req_flush_cnt = 0;
        iv.flush_cnt = ~0ul;
        iv.cache_line_i = 0;
    }

    sc_signal<bool> line_direct_access_i;
    sc_signal<bool> line_invalidate_i;
    sc_signal<bool> line_re_i;
    sc_signal<bool> line_we_i;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> line_addr_i;
    sc_signal<sc_biguint<ICACHE_LINE_BITS>> line_wdata_i;
    sc_signal<sc_uint<(1<<CFG_ILOG2_BYTES_PER_LINE)>> line_wstrb_i;
    sc_signal<sc_uint<ITAG_FL_TOTAL>> line_wflags_i;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> line_raddr_o;
    sc_signal<sc_biguint<ICACHE_LINE_BITS+16>> line_rdata_o;
    sc_signal<sc_uint<ITAG_FL_TOTAL>> line_rflags_o;
    sc_signal<bool> line_hit_o;
    sc_signal<bool> line_hit_next_o;

    TagMemCoupled<CFG_CPU_ADDR_BITS,
            CFG_ILOG2_NWAYS,
            CFG_ILOG2_LINES_PER_WAY,
            CFG_ILOG2_BYTES_PER_LINE,
            ITAG_FL_TOTAL> *memcouple;

    bool async_reset_;
};


#ifdef DBG_ICACHE_LRU_TB
SC_MODULE(ICacheLru_tb) {
    void comb0();
    void comb_fetch();
    void comb_bus();
    void registers() {
        r = v;
        rbus = vbus;
    }

    SC_HAS_PROCESS(ICacheLru_tb);

    ICacheLru_tb(sc_module_name name_);

private:
    ICacheLru *tt;

    sc_clock w_clk;
    sc_signal<bool> w_nrst;
    // Control path:
    sc_signal<bool> w_req_ctrl_valid;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_req_ctrl_addr;
    sc_signal<bool> w_req_ctrl_ready;
    sc_signal<bool> w_resp_ctrl_valid;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_resp_ctrl_addr;
    sc_signal<sc_uint<32>> wb_resp_ctrl_data;
    sc_signal<bool> w_resp_ctrl_load_fault;
    sc_signal<bool> w_resp_ctrl_ready;
    // Memory interface:
    sc_signal<bool> w_req_mem_ready;
    sc_signal<bool> w_req_mem_valid;
    sc_signal<bool> w_req_mem_write;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_req_mem_addr;
    sc_signal<sc_uint<BUS_DATA_BYTES>> wb_req_mem_strob;
    sc_signal<sc_uint<BUS_DATA_WIDTH>> wb_req_mem_data;
    sc_signal<bool> w_resp_mem_data_valid;
    sc_signal<sc_uint<BUS_DATA_WIDTH>> wb_resp_mem_data;
    sc_signal<bool> w_resp_mem_load_fault;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_flush_address;
    sc_signal<bool> w_flush_valid;
    sc_signal<sc_uint<2>> wb_istate;

    struct RegistersType {
        sc_signal<sc_uint<32>> clk_cnt;
    } v, r;

    enum EBusState {
        BUS_Idle,
        BUS_Read,
        BUS_ReadLast
    };

    struct BusRegistersType {
        sc_signal<sc_uint<2>> state;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> burst_addr;
        sc_signal<sc_uint<2>> burst_cnt;
    } vbus, rbus;

    sc_trace_file *tb_vcd;
};
#endif  // DBG_ICACHE_LRU_TB

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_ICACHE_LRU_H__
