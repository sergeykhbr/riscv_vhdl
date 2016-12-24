/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Instruction Cache.
 */

#ifndef __DEBUGGER_RIVERLIB_ICACHE_H__
#define __DEBUGGER_RIVERLIB_ICACHE_H__

#include <systemc.h>
#include "../river_cfg.h"

namespace debugger {

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

    struct RegistersType {
        sc_signal<sc_uint<BUS_ADDR_WIDTH - 3>> iline_addr;
        sc_signal<sc_uint<BUS_DATA_WIDTH>> iline_data;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> iline_addr_req;
        sc_signal<sc_uint<2>> state;
    } v, r;

};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_ICACHE_H__
