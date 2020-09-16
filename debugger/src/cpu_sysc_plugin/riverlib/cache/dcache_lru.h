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

#ifndef __DEBUGGER_RIVERLIB_DCACHE_LRU_H__
#define __DEBUGGER_RIVERLIB_DCACHE_LRU_H__

#include <systemc.h>
#include "../river_cfg.h"
#include "tagmemnway.h"

namespace debugger {

//#define DBG_DCACHE_LRU_TB

SC_MODULE(DCacheLru) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    // Control path:
    sc_in<bool> i_req_valid;
    sc_in<bool> i_req_write;
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_req_addr;
    sc_in<sc_uint<64>> i_req_wdata;
    sc_in<sc_uint<8>> i_req_wstrb;
    sc_in<sc_uint<2>> i_req_size;
    sc_out<bool> o_req_ready;
    sc_out<bool> o_resp_valid;
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_resp_addr;
    sc_out<sc_uint<64>> o_resp_data;
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_resp_er_addr;
    sc_out<bool> o_resp_er_load_fault;
    sc_out<bool> o_resp_er_store_fault;
    sc_out<bool> o_resp_er_mpu_load;        // No access rights to read/execute
    sc_out<bool> o_resp_er_mpu_store;       // No access rights to write
    sc_in<bool> i_resp_ready;
    // Memory interface:
    sc_in<bool> i_req_mem_ready;
    sc_out<bool> o_req_mem_valid;
    sc_out<sc_uint<REQ_MEM_TYPE_BITS>> o_req_mem_type;
    sc_out<sc_uint<3>> o_req_mem_size;
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_req_mem_addr;
    sc_out<sc_uint<DCACHE_BYTES_PER_LINE>> o_req_mem_strob;
    sc_out<sc_biguint<DCACHE_LINE_BITS>> o_req_mem_data;
    sc_in<bool> i_mem_data_valid;
    sc_in<sc_biguint<DCACHE_LINE_BITS>> i_mem_data;
    sc_in<bool> i_mem_load_fault;
    sc_in<bool> i_mem_store_fault;
    // Mpu interface
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_mpu_addr;
    sc_in<sc_uint<CFG_MPU_FL_TOTAL>> i_mpu_flags;
    // D$ Snoop interface
    sc_in<bool> i_req_snoop_valid;
    sc_in<sc_uint<SNOOP_REQ_TYPE_BITS>> i_req_snoop_type;
    sc_out<bool> o_req_snoop_ready;
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> i_req_snoop_addr;
    sc_in<bool> i_resp_snoop_ready;
    sc_out<bool> o_resp_snoop_valid;
    sc_out<sc_biguint<L1CACHE_LINE_BITS>> o_resp_snoop_data;
    sc_out<sc_uint<DTAG_FL_TOTAL>> o_resp_snoop_flags;
    // Debug interface
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_flush_address;
    sc_in<bool> i_flush_valid;
    sc_out<bool> o_flush_end;

    void comb();
    void registers();

    SC_HAS_PROCESS(DCacheLru);

    DCacheLru(sc_module_name name_, bool async_reset, bool coherence_ena);
    virtual ~DCacheLru();

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
        State_WriteBus,
        State_FlushAddr,
        State_FlushCheck,
        State_Reset,
        State_ResetWrite,
        State_SnoopSetupAddr,
        State_SnoopReadData,
    };

    sc_signal<bool> line_direct_access_i;
    sc_signal<bool> line_invalidate_i;
    sc_signal<bool> line_re_i;
    sc_signal<bool> line_we_i;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> line_addr_i;
    sc_signal<sc_biguint<DCACHE_LINE_BITS>> line_wdata_i;
    sc_signal<sc_uint<DCACHE_BYTES_PER_LINE>> line_wstrb_i;
    sc_signal<sc_uint<DTAG_FL_TOTAL>> line_wflags_i;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> line_raddr_o;
    sc_signal<sc_biguint<DCACHE_LINE_BITS>> line_rdata_o;
    sc_signal<sc_uint<DTAG_FL_TOTAL>> line_rflags_o;
    sc_signal<bool> line_hit_o;
    // Snoop signals:
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> line_snoop_addr_i;
    sc_signal<bool> line_snoop_ready_o;
    sc_signal<sc_uint<DTAG_FL_TOTAL>> line_snoop_flags_o;


    struct RegistersType {
        sc_signal<bool> req_write;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_addr;
        sc_signal<sc_uint<64>> req_wdata;
        sc_signal<sc_uint<8>> req_wstrb;
        sc_signal<sc_uint<3>> req_size;
        sc_signal<sc_uint<4>> state;
        sc_signal<bool> req_mem_valid;
        sc_signal<sc_uint<REQ_MEM_TYPE_BITS>> req_mem_type;
        sc_signal<sc_uint<3>> req_mem_size;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> mem_addr;
        sc_signal<bool> mpu_er_store;
        sc_signal<bool> mpu_er_load;
        sc_signal<bool> load_fault;
        sc_signal<bool> write_first;
        sc_signal<bool> write_flush;
        sc_signal<bool> write_share;
        sc_signal<sc_uint<DCACHE_BYTES_PER_LINE>> mem_wstrb;
        sc_signal<bool> req_flush;
        sc_signal<bool> req_flush_all;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_flush_addr;
        sc_signal<sc_uint<CFG_DLOG2_LINES_PER_WAY + CFG_DLOG2_NWAYS>> req_flush_cnt;
        sc_signal<sc_uint<CFG_DLOG2_LINES_PER_WAY + CFG_DLOG2_NWAYS>> flush_cnt;
        sc_signal<sc_biguint<DCACHE_LINE_BITS>> cache_line_i;
        sc_signal<sc_biguint<DCACHE_LINE_BITS>> cache_line_o;
        sc_signal<sc_uint<SNOOP_REQ_TYPE_BITS>> req_snoop_type;
        sc_signal<bool> snoop_flags_valid;
        sc_signal<bool> snoop_restore_wait_resp;
        sc_signal<bool> snoop_restore_write_bus;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_addr_restore;
    } v, r;

    void R_RESET(RegistersType &iv) {
        iv.req_write = 0;
        iv.req_addr = 0;
        iv.req_wdata = 0;
        iv.req_wstrb = 0;
        iv.req_size = 0;
        iv.state = State_Reset;
        iv.req_mem_valid = 0;
        iv.req_mem_type = 0;
        iv.req_mem_size = 0;
        iv.mem_addr = 0;
        iv.mpu_er_store = 0;
        iv.mpu_er_load = 0;
        iv.load_fault = 0;
        iv.write_first = 0;
        iv.write_flush = 0;
        iv.write_share = 0;
        iv.mem_wstrb = 0;
        iv.req_flush = 0;           // init flush request
        iv.req_flush_all = 0;
        iv.req_flush_addr = 0;      // [0]=1 flush all
        iv.req_flush_cnt = 0;
        iv.flush_cnt = ~0ul;
        iv.cache_line_i = 0;
        iv.cache_line_o = 0;
        iv.req_snoop_type = 0;
        iv.snoop_flags_valid = 0;
        iv.snoop_restore_wait_resp = 0;
        iv.snoop_restore_write_bus = 0;
        iv.req_addr_restore = 0;
    }

    static const int CFG_SNOOP_ENA = 1;
    TagMemNWay<CFG_CPU_ADDR_BITS,
               CFG_DLOG2_NWAYS,
               CFG_DLOG2_LINES_PER_WAY,
               CFG_DLOG2_BYTES_PER_LINE,
               DTAG_FL_TOTAL,
               CFG_SNOOP_ENA> *mem;

    bool async_reset_;
    bool coherence_ena_;
};


#ifdef DBG_DCACHE_LRU_TB
SC_MODULE(DCacheLru_tb) {
    void comb0();
    void comb_fetch();
    void comb_bus();
    void registers() {
        r = v;
        rbus = vbus;
    }

    SC_HAS_PROCESS(DCacheLru_tb);

    DCacheLru_tb(sc_module_name name_);

private:
    DCacheLru *tt;

    sc_clock w_clk;
    sc_signal<bool> w_nrst;
    // Control path:
    sc_signal<bool> w_req_valid;
    sc_signal<bool> w_req_write;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_req_addr;
    sc_signal<sc_uint<64>> wb_req_wdata;
    sc_signal<sc_uint<8>> wb_req_wstrb;
    sc_signal<bool> w_req_ready;
    sc_signal<bool> w_resp_valid;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_resp_addr;
    sc_signal<sc_uint<64>> wb_resp_data;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_resp_er_addr;
    sc_signal<bool> w_resp_er_load_fault;
    sc_signal<bool> w_resp_er_store_fault;
    sc_signal<bool> w_resp_er_mpu_load;
    sc_signal<bool> w_resp_er_mpu_store;
    sc_signal<bool> w_resp_ready;
    // Memory interface:
    sc_signal<bool> w_req_mem_ready;
    sc_signal<bool> w_req_mem_valid;
    sc_signal<bool> w_req_mem_write;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_req_mem_addr;
    sc_signal<sc_uint<8>> wb_req_mem_strob;
    sc_signal<sc_uint<64>> wb_req_mem_data;
    sc_signal<sc_uint<8>> wb_req_mem_len;       // burst transactions num
    sc_signal<sc_uint<2>> wb_req_mem_burst;     // "01" INCR; "10" burst WRAP
    sc_signal<bool> w_req_mem_last;            // last in sequence flag
    sc_signal<bool> w_mem_data_valid;
    sc_signal<sc_uint<64>> wb_mem_data;
    sc_signal<bool> w_mem_load_fault;
    sc_signal<bool> w_mem_store_fault;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_mpu_addr;
    sc_signal<sc_uint<CFG_MPU_FL_TOTAL>> w_mpu_flags;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> wb_flush_address;
    sc_signal<bool> w_flush_valid;
    sc_signal<sc_uint<4>> wb_state;

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
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> mpu_addr;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> burst_addr;
        sc_signal<sc_uint<8>> burst_cnt;
    } vbus, rbus;

    sc_trace_file *tb_vcd;
};
#endif  // DBG_DCACHE_LRU_TB

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_DCACHE_LRU_H__
