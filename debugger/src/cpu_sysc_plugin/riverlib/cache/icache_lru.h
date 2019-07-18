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

//#define DBG_ICACHE_TB

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

    ICacheLru(sc_module_name name_, bool async_reset, int isize);
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
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> req_addr;
        sc_signal<bool> use_overlay;
        sc_signal<bool> x_removed;
        sc_signal<sc_uint<2>> state;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> mem_addr;
        sc_signal<sc_uint<2>> burst_cnt;
        sc_signal<sc_uint<4>> burst_wstrb;
        sc_signal<sc_uint<4>> burst_valid;
    } v, r;

    void R_RESET(RegistersType &iv) {
        iv.req_addr = 0;
        iv.use_overlay = 0;
        iv.x_removed = 0;
        iv.state = State_Idle;
        iv.mem_addr = 0;
        iv.burst_cnt = 0;
        iv.burst_wstrb = 0;
        iv.burst_valid = 0;
    }

    IWayMem *wayevenx[CFG_ICACHE_WAYS];
    IWayMem *wayoddx[CFG_ICACHE_WAYS];

    ILru *lrueven;
    ILru *lruodd;

    TagMemInType swapin[WAY_SubNum];
    TagMemOutType memeven[WAY_SubNum];
    TagMemOutType memodd[WAY_SubNum];
    WayMuxType waysel[WAY_SubNum];
    sc_signal<bool> wb_ena_even[CFG_ICACHE_WAYS];
    sc_signal<bool> wb_ena_odd[CFG_ICACHE_WAYS];

    LruInType lrui[WAY_SubNum];
    sc_signal<sc_uint<2>> wb_lru_even;
    sc_signal<sc_uint<2>> wb_lru_odd;

    bool async_reset_;
    int isize_;
};


#ifdef DBG_ICACHE_TB
SC_MODULE(ICacheLru_tb) {
    void comb0();
    void registers() {
        r = v;
    }

    SC_HAS_PROCESS(ICacheLru_tb);

    ICacheLru_tb(sc_module_name name_) : sc_module(name_),
        w_clk("clk0", 10, SC_NS) {
        SC_METHOD(comb0);
        sensitive << w_nrst;
        sensitive << w_req_ctrl_valid;
        sensitive << wb_req_ctrl_addr;
        sensitive << w_req_ctrl_ready;
        sensitive << w_resp_ctrl_valid;
        sensitive << wb_resp_ctrl_addr;
        sensitive << wb_resp_ctrl_data;
        sensitive << w_resp_ctrl_ready;
        sensitive << w_req_mem_ready;
        sensitive << w_req_mem_valid;
        sensitive << w_req_mem_write;
        sensitive << wb_req_mem_addr;
        sensitive << wb_req_mem_strob;
        sensitive << wb_req_mem_data;
        sensitive << w_resp_mem_data_valid;
        sensitive << wb_resp_mem_data;
        sensitive << wb_istate;
        sensitive << r.clk_cnt;
        sensitive << r.mem_raddr;
        sensitive << r.mem_state;
        sensitive << r.mem_cnt;
        sensitive << r.mem_wait_cnt;
        sensitive << r.fetch_state;
        sensitive << r.fetch_cnt;

        SC_METHOD(registers);
        sensitive << w_clk.posedge_event();

        tt = new ICacheLru("tt");
        tt->i_clk(w_clk);
        tt->i_nrst(w_nrst);
        tt->i_req_ctrl_valid(w_req_ctrl_valid);
        tt->i_req_ctrl_addr(wb_req_ctrl_addr);
        tt->o_req_ctrl_ready(w_req_ctrl_ready);
        tt->o_resp_ctrl_valid(w_resp_ctrl_valid);
        tt->o_resp_ctrl_addr(wb_resp_ctrl_addr);
        tt->o_resp_ctrl_data(wb_resp_ctrl_data);
        tt->i_resp_ctrl_ready(w_resp_ctrl_ready);
        tt->i_req_mem_ready(w_req_mem_ready);
        tt->o_req_mem_valid(w_req_mem_valid);
        tt->o_req_mem_write(w_req_mem_write);
        tt->o_req_mem_addr(wb_req_mem_addr);
        tt->o_req_mem_strob(wb_req_mem_strob);
        tt->o_req_mem_data(wb_req_mem_data);
        tt->i_resp_mem_data_valid(w_resp_mem_data_valid);
        tt->i_resp_mem_data(wb_resp_mem_data);
        tt->o_istate(wb_istate);

        tb_vcd = sc_create_vcd_trace_file("ICacheLru_tb");
        tb_vcd->set_time_unit(1, SC_PS);
        sc_trace(tb_vcd, w_nrst, "w_nrst");
        sc_trace(tb_vcd, w_clk, "w_clk");
        sc_trace(tb_vcd, r.clk_cnt, "clk_cnt");
        sc_trace(tb_vcd, w_req_ctrl_valid, "w_req_ctrl_valid");
        sc_trace(tb_vcd, wb_req_ctrl_addr, "wb_req_ctrl_addr");
        sc_trace(tb_vcd, w_req_ctrl_ready, "w_req_ctrl_ready");
        sc_trace(tb_vcd, w_resp_ctrl_valid, "w_resp_ctrl_valid");
        sc_trace(tb_vcd, wb_resp_ctrl_addr, "wb_resp_ctrl_addr");
        sc_trace(tb_vcd, wb_resp_ctrl_data, "wb_resp_ctrl_data");
        sc_trace(tb_vcd, w_resp_ctrl_ready, "w_resp_ctrl_ready");
        sc_trace(tb_vcd, w_req_mem_ready, "w_req_mem_ready");
        sc_trace(tb_vcd, w_req_mem_valid, "w_req_mem_valid");
        sc_trace(tb_vcd, w_req_mem_write, "w_req_mem_write");
        sc_trace(tb_vcd, wb_req_mem_addr, "wb_req_mem_addr");
        sc_trace(tb_vcd, wb_req_mem_strob, "wb_req_mem_strob");
        sc_trace(tb_vcd, wb_req_mem_data, "wb_req_mem_data");
        sc_trace(tb_vcd, w_resp_mem_data_valid, "w_resp_mem_data_valid");
        sc_trace(tb_vcd, wb_resp_mem_data, "wb_resp_mem_data");
        sc_trace(tb_vcd, wb_istate, "wb_istate");
        sc_trace(tb_vcd, wb_istate_z, "wb_istate_z");
        sc_trace(tb_vcd, w_ierr_state, "w_ierr_state");
        sc_trace(tb_vcd, r.mem_state, "r_mem_state");
        sc_trace(tb_vcd, r.mem_raddr, "r_mem_raddr");

        tt->generateVCD(tb_vcd, tb_vcd);
    }

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
    sc_signal<sc_uint<2>> wb_istate;

    struct RegistersType {
        sc_signal<sc_uint<32>> clk_cnt;
        sc_signal<sc_uint<2>> fetch_state;
        sc_signal<sc_uint<8>> fetch_cnt;
        sc_signal<sc_uint<8>> fetch_wait_cnt;
        sc_signal<sc_uint<2>> mem_state;
        sc_signal<sc_uint<32>> mem_raddr;
        sc_signal<sc_uint<8>> mem_cnt;
        sc_signal<sc_uint<8>> mem_wait_cnt;
    } v, r;
    sc_trace_file *tb_vcd;
};
#endif  // DBG_ICACHE_TB

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_ICACHE_LRU_H__
