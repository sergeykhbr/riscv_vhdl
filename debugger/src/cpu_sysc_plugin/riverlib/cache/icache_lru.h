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
#include "iwaymem.h"
#include "ilru.h"

namespace debugger {

#define DBG_ICACHE_LRU_TB

SC_MODULE(ICacheLru) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    // Control path:
    sc_in<bool> i_req_ctrl_valid;
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_req_ctrl_addr;
    sc_out<bool> o_req_ctrl_ready;
    sc_out<bool> o_resp_ctrl_valid;
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_resp_ctrl_addr;
    sc_out<sc_uint<32>> o_resp_ctrl_data;
    sc_out<bool> o_resp_ctrl_load_fault;
    sc_in<bool> i_resp_ctrl_ready;
    // Memory interface:
    sc_in<bool> i_req_mem_ready;
    sc_out<bool> o_req_mem_valid;
    sc_out<bool> o_req_mem_write;
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_req_mem_addr;
    sc_out<sc_uint<BUS_DATA_BYTES>> o_req_mem_strob;
    sc_out<sc_uint<BUS_DATA_WIDTH>> o_req_mem_data;
    sc_in<bool> i_resp_mem_data_valid;
    sc_in<sc_uint<BUS_DATA_WIDTH>> i_resp_mem_data;
    sc_in<bool> i_resp_mem_load_fault;
    // Debug interface
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_flush_address;
    sc_in<bool> i_flush_valid;
    sc_out<sc_uint<2>> o_istate;

    void comb();
    void registers();

    SC_HAS_PROCESS(ICacheLru);

    ICacheLru(sc_module_name name_, bool async_reset, int ilines_per_way);
    virtual ~ICacheLru();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    static const uint8_t MISS = static_cast<uint8_t>(CFG_ICACHE_WAYS);
    enum EWays {
        WAY_EVEN,
        WAY_ODD,
        WAY_SubNum
    };

    enum EState {
        State_Idle,
        State_CheckHit,
        State_WaitGrant,
        State_WaitResp,
        State_CheckResp,
        State_Flush
    };

    struct TagMemInType {
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> radr;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> wadr;
        sc_signal<sc_uint<4>> wstrb;
        sc_signal<sc_uint<4>> wvalid;
        sc_signal<sc_uint<64>> wdata;
        sc_signal<bool> load_fault;
    };

    struct TagMemOutType {
        sc_signal<sc_uint<CFG_ITAG_WIDTH>> rtag;
        sc_signal<sc_uint<32>> rdata;
        sc_signal<bool> valid;
        sc_signal<bool> load_fault;
    };

    struct WayMuxType {
        sc_uint<3> hit;
        sc_uint<32> rdata;
        bool valid;
        bool load_fault;
    };

    struct LruInType {
        sc_signal<sc_uint<CFG_IINDEX_WIDTH>> adr;
        sc_signal<bool> we;
        sc_signal<sc_uint<2>> lru;
    };

    struct RegistersType {
        sc_signal<bool> init_done;
        sc_signal<bool> requested;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> req_addr;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> req_addr_overlay;
        sc_signal<bool> use_overlay;
        sc_signal<sc_uint<3>> state;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> mem_addr;
        sc_signal<sc_uint<2>> burst_cnt;
        sc_signal<sc_uint<4>> burst_wstrb;
        sc_signal<sc_uint<4>> burst_valid;
        sc_signal<sc_uint<2>> lru_even_wr;
        sc_signal<sc_uint<2>> lru_odd_wr;
        sc_signal<sc_uint<CFG_IINDEX_WIDTH>> flush_cnt;
    } v, r;

    void R_RESET(RegistersType &iv) {
        iv.init_done = 0;
        iv.requested = 0;
        iv.req_addr = 0;
        iv.req_addr_overlay = 0;
        iv.use_overlay = 0;
        iv.state = State_Idle;
        iv.mem_addr = 0;
        iv.burst_cnt = 0;
        iv.burst_wstrb = 0;
        iv.burst_valid = 0;
        iv.lru_even_wr = 0;
        iv.lru_odd_wr = 0;
        iv.flush_cnt = 0;
    }

    IWayMem *wayevenx[CFG_ICACHE_WAYS];
    IWayMem *wayoddx[CFG_ICACHE_WAYS];

    ILru *lrueven;
    ILru *lruodd;

    TagMemInType swapin[WAY_SubNum];
    TagMemOutType memeven[CFG_ICACHE_WAYS];
    TagMemOutType memodd[CFG_ICACHE_WAYS];
    WayMuxType waysel[WAY_SubNum];
    sc_signal<bool> wb_ena_even[CFG_ICACHE_WAYS];
    sc_signal<bool> wb_ena_odd[CFG_ICACHE_WAYS];

    LruInType lrui[WAY_SubNum];
    sc_signal<sc_uint<2>> wb_lru_even;
    sc_signal<sc_uint<2>> wb_lru_odd;

    bool async_reset_;
    int ilines_per_way_;
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
    sc_signal<sc_uint<BUS_ADDR_WIDTH>> wb_req_ctrl_addr;
    sc_signal<bool> w_req_ctrl_ready;
    sc_signal<bool> w_resp_ctrl_valid;
    sc_signal<sc_uint<BUS_ADDR_WIDTH>> wb_resp_ctrl_addr;
    sc_signal<sc_uint<32>> wb_resp_ctrl_data;
    sc_signal<bool> w_resp_ctrl_load_fault;
    sc_signal<bool> w_resp_ctrl_ready;
    // Memory interface:
    sc_signal<bool> w_req_mem_ready;
    sc_signal<bool> w_req_mem_valid;
    sc_signal<bool> w_req_mem_write;
    sc_signal<sc_uint<BUS_ADDR_WIDTH>> wb_req_mem_addr;
    sc_signal<sc_uint<BUS_DATA_BYTES>> wb_req_mem_strob;
    sc_signal<sc_uint<BUS_DATA_WIDTH>> wb_req_mem_data;
    sc_signal<bool> w_resp_mem_data_valid;
    sc_signal<sc_uint<BUS_DATA_WIDTH>> wb_resp_mem_data;
    sc_signal<bool> w_resp_mem_load_fault;
    sc_signal<sc_uint<BUS_ADDR_WIDTH>> wb_flush_address;
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
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> burst_addr;
        sc_signal<sc_uint<2>> burst_cnt;
    } vbus, rbus;

    sc_trace_file *tb_vcd;
};
#endif  // DBG_ICACHE_LRU_TB

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_ICACHE_LRU_H__
