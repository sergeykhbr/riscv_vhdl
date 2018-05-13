/**
 * @file
 * @copyright  Copyright 2017 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Instruction Cache.
 */

#ifndef __DEBUGGER_RIVERLIB_ICACHE_H__
#define __DEBUGGER_RIVERLIB_ICACHE_H__

#include <systemc.h>
#include "../river_cfg.h"

namespace debugger {

//#define DBG_ICACHE_TB

SC_MODULE(ICache) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    // Control path:
    sc_in<bool> i_req_ctrl_valid;
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_req_ctrl_addr;
    sc_out<bool> o_req_ctrl_ready;
    sc_out<bool> o_resp_ctrl_valid;
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_resp_ctrl_addr;
    sc_out<sc_uint<32>> o_resp_ctrl_data;
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
    sc_out<sc_uint<2>> o_istate;

    void comb();
    void registers();

    SC_HAS_PROCESS(ICache);

    ICache(sc_module_name name_);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

private:
    enum EState {
        State_Idle,
        State_WaitGrant,
        State_WaitResp,
        State_WaitAccept
    };
    enum EHit {
        Hit_Line1,
        Hit_Line2,
        Hit_Response,
        Hit_Total
    };
    static const int ILINE_TOTAL = 2;

    struct RegistersType {
        sc_signal<sc_uint<BUS_ADDR_WIDTH - 3>> iline_addr[ILINE_TOTAL];
        sc_signal<sc_uint<BUS_DATA_WIDTH>> iline_data[ILINE_TOTAL];
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> iline_addr_req;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> addr_processing;
        sc_signal<sc_uint<2>> state;
        sc_signal<bool> double_req;         // request 2-lines
        sc_signal<bool> delay_valid;
        sc_signal<sc_uint<32>> delay_data;
    } v, r;
    bool w_need_mem_req;
    sc_uint<32> hit_word;
    sc_bv<ILINE_TOTAL + 1> wb_hit[ILINE_TOTAL];
    sc_bv<ILINE_TOTAL> wb_hit_hold[ILINE_TOTAL];
    bool w_check;
    sc_uint<BUS_DATA_WIDTH> wb_hit_data[ILINE_TOTAL];
    sc_uint<BUS_DATA_WIDTH> wb_hold_data[ILINE_TOTAL];
};


#ifdef DBG_ICACHE_TB
SC_MODULE(ICache_tb) {
    void comb0();
    void registers() {
        r = v;
    }

    SC_HAS_PROCESS(ICache_tb);

    ICache_tb(sc_module_name name_) : sc_module(name_),
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

        tt = new ICache("tt");
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

        tb_vcd = sc_create_vcd_trace_file("icache_tb");
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
    ICache *tt;

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

#endif  // __DEBUGGER_RIVERLIB_ICACHE_H__
