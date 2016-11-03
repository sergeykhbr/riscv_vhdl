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
    sc_in<sc_uint<AXI_ADDR_WIDTH>> i_req_data_addr;
    sc_in<sc_uint<RISCV_ARCH>> i_req_data_data;
    sc_out<bool> o_resp_data_valid;
    sc_out<sc_uint<AXI_ADDR_WIDTH>> o_resp_data_addr;
    sc_out<sc_uint<RISCV_ARCH>> o_resp_data_data;
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

    SC_HAS_PROCESS(DCache);

    DCache(sc_module_name name_, sc_trace_file *vcd=0);

private:
    struct RegistersType {
        sc_signal<sc_uint<AXI_ADDR_WIDTH>> req_addr;
        sc_signal<sc_uint<2>> req_size;
        sc_signal<bool> rena;
    } v, r;

    sc_uint<AXI_ADDR_WIDTH> wb_req_addr;
    sc_uint<AXI_DATA_BYTES> wb_req_strob;
    sc_uint<AXI_DATA_WIDTH> wb_rdata;
    sc_uint<AXI_DATA_WIDTH> wb_wdata;
    sc_uint<AXI_DATA_WIDTH> wb_rtmp;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_DCACHE_H__
