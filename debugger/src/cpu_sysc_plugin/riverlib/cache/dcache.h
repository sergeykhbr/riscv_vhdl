/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Data Cache.
 */

#ifndef __DEBUGGER_RIVERLIB_DCACHE_H__
#define __DEBUGGER_RIVERLIB_DCACHE_H__

#include <systemc.h>
#include "../river_cfg.h"

namespace debugger {

SC_MODULE(DCache) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    // Data path:
    sc_in<bool> i_req_data_valid;
    sc_in<bool> i_req_data_write;
    sc_in<sc_uint<2>> i_req_data_sz;
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_req_data_addr;
    sc_in<sc_uint<RISCV_ARCH>> i_req_data_data;
    sc_out<bool> o_req_data_ready;
    sc_out<bool> o_resp_data_valid;
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_resp_data_addr;
    sc_out<sc_uint<RISCV_ARCH>> o_resp_data_data;
    sc_out<bool> o_resp_data_load_fault;
    sc_out<bool> o_resp_data_store_fault;
    sc_in<bool> i_resp_data_ready;
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
    sc_in<bool> i_resp_mem_store_fault;
    sc_out<sc_uint<2>> o_dstate;

    void comb();
    void registers();

    SC_HAS_PROCESS(DCache);

    DCache(sc_module_name name_, bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

private:
    enum EState {
        State_Idle,
        State_WaitGrant,
        State_WaitResp,
        State_WaitAccept
    };

    struct RegistersType {
        sc_signal<sc_uint<BUS_DATA_BYTES>> req_strob;
        sc_signal<sc_uint<BUS_DATA_WIDTH>> req_wdata;
        sc_signal<sc_uint<BUS_DATA_WIDTH>> dline_data;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> dline_addr_req;
        sc_signal<sc_uint<2>> dline_size_req;
        sc_signal<bool> dline_load_fault;
        sc_signal<bool> dline_store_fault;
        sc_signal<sc_uint<2>> state;
    } v, r;

    void R_RESET(RegistersType &iv) {
        iv.req_strob = 0;
        iv.req_wdata = 0;
        iv.dline_data = 0;
        iv.dline_addr_req = 0;
        iv.dline_size_req = 0;
        iv.dline_load_fault = 0;
        iv.dline_store_fault = 0;
        iv.state = State_Idle;
    }

    bool w_wait_response;
    bool async_reset_;
};

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_DCACHE_H__
