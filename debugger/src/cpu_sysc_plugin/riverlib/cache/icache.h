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
    sc_out<bool> o_req_ctrl_ready;
    sc_in<sc_uint<AXI_ADDR_WIDTH>> i_req_ctrl_addr;
    sc_out<bool> o_resp_ctrl_valid;
    sc_out<sc_uint<AXI_ADDR_WIDTH>> o_resp_ctrl_addr;
    sc_out<sc_uint<32>> o_resp_ctrl_data;
    // Memory interface:
    sc_out<bool> o_req_mem_valid;
    sc_out<bool> o_req_mem_write;
    sc_out<sc_uint<AXI_ADDR_WIDTH>> o_req_mem_addr;
    sc_out<sc_uint<AXI_DATA_BYTES>> o_req_mem_strob;
    sc_out<sc_uint<AXI_DATA_WIDTH>> o_req_mem_data;
    sc_in<bool> i_resp_mem_data_valid;
    sc_in<sc_uint<AXI_DATA_WIDTH>> i_resp_mem_data;


    void comb();
    void registers();

    SC_HAS_PROCESS(ICache);

    ICache(sc_module_name name_, sc_trace_file *vcd=0);

private:
    struct RegistersType {
        sc_signal<sc_uint<AXI_ADDR_WIDTH - 3>> iline_addr;
        sc_signal<sc_uint<AXI_DATA_WIDTH>> iline_data;

        sc_uint<AXI_ADDR_WIDTH> iline_addr_req;
        bool ihit;
        sc_uint<32> ihit_data;
    } v, r;

    sc_uint<AXI_ADDR_WIDTH - 3> wb_req_line;
    sc_uint<AXI_ADDR_WIDTH - 3> wb_cached_addr;
    sc_uint<AXI_DATA_WIDTH> wb_cached_data;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_ICACHE_H__
