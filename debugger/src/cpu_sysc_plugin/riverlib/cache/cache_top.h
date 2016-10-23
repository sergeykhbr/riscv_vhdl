/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      Memory Cache Top level.
 */

#ifndef __DEBUGGER_RIVERLIB_CACHE_TOP_H__
#define __DEBUGGER_RIVERLIB_CACHE_TOP_H__

#include <systemc.h>
#include "../river_cfg.h"

namespace debugger {

SC_MODULE(CacheTop) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    // Control path:
    sc_in<bool> i_req_ctrl_valid;
    sc_in<sc_uint<AXI_ADDR_WIDTH>> i_req_ctrl_addr;
    sc_out<bool> o_resp_ctrl_ready;
    sc_out<sc_uint<32>> o_resp_ctrl_data;
    // Data path:
    sc_in<bool> i_req_data_valid;
    sc_in<bool> i_req_data_write;
    sc_in<sc_uint<AXI_ADDR_WIDTH>> i_req_data_addr;
    sc_in<sc_uint<2>> i_req_data_size; // 0=1bytes; 1=2bytes; 2=4bytes; 3=8bytes
    sc_in<sc_uint<RISCV_ARCH>> i_req_data_data;
    sc_out<bool> o_resp_data_ready;
    sc_out<sc_uint<RISCV_ARCH>> o_resp_data_data;
    // Memory interface:
    sc_out<bool> o_req_mem_valid;
    sc_out<bool> o_req_mem_write;
    sc_out<sc_uint<AXI_ADDR_WIDTH>> o_req_mem_addr;
    sc_out<sc_uint<AXI_DATA_BYTES>> o_req_mem_strob;
    sc_out<sc_uint<AXI_DATA_WIDTH>> o_req_mem_data;
    sc_in<bool> i_resp_mem_ready;
    sc_in<sc_uint<AXI_DATA_WIDTH>> i_resp_mem_data;


    void proc0();
    void registers();

    SC_HAS_PROCESS(CacheTop);

    CacheTop(sc_module_name name_, sc_trace_file *vcd=0);

private:
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_CACHE_TOP_H__
