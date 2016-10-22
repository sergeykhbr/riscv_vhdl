/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      "River" CPU Top level.
 */

#ifndef __DEBUGGER_RIVER_TOP_H__
#define __DEBUGGER_RIVER_TOP_H__

#include <systemc.h>
#include "river_cfg.h"

namespace debugger {

SC_MODULE(RiverTop) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    // Timer:
    sc_out<sc_uint<RISCV_ARCH>> o_timer;
    // Memory interface:
    sc_out<bool> o_req_mem_valid;
    sc_out<bool> o_req_mem_write;
    sc_out<sc_uint<AXI_ADDR_WIDTH>> o_req_mem_addr;
    sc_out<sc_uint<AXI_DATA_BYTES>> o_req_mem_strob;
    sc_out<sc_uint<AXI_DATA_WIDTH>> o_req_mem_data;
    sc_in<bool> i_resp_mem_ready;
    sc_in<sc_uint<AXI_DATA_WIDTH>> i_resp_mem_data;


    uint64_t wb_pc;
    sc_signal<sc_uint<AXI_ADDR_WIDTH>> rb_pc;
    sc_signal<sc_uint<RISCV_ARCH>> rb_timer;

    void proc0();
    void registers();

    SC_HAS_PROCESS(RiverTop);

    RiverTop(sc_module_name name_, sc_trace_file *vcd=0);
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVER_TOP_H__
