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
#include "core/proc.h"
#include "cache/cache_top.h"

namespace debugger {

SC_MODULE(RiverTop) {
    sc_in<bool> i_clk;                                  // CPU clock
    sc_in<bool> i_nrst;                                 // Reset: active LOW
    // Timer:
    sc_out<sc_uint<RISCV_ARCH>> o_timer;                // todo: move to debug interface
    // Memory interface:
    sc_out<bool> o_req_mem_valid;                       // AXI memory request is valid
    sc_out<bool> o_req_mem_write;                       // AXI memory request is write type
    sc_out<sc_uint<AXI_ADDR_WIDTH>> o_req_mem_addr;     // AXI memory request address
    sc_out<sc_uint<AXI_DATA_BYTES>> o_req_mem_strob;    // Writing strob. 1 bit per Byte
    sc_out<sc_uint<AXI_DATA_WIDTH>> o_req_mem_data;     // Writing data
    sc_in<bool> i_resp_mem_data_valid;                  // AXI response is valid
    sc_in<sc_uint<AXI_DATA_WIDTH>> i_resp_mem_data;     // Read data
    /** Interrupt line from external interrupts controller (PLIC). */
    sc_in<bool> i_ext_irq;


    void comb();
    void registers();

    SC_HAS_PROCESS(RiverTop);

    RiverTop(sc_module_name name_, sc_trace_file *vcd=0);
    virtual ~RiverTop();

private:

    Processor *proc0;
    CacheTop *cache0;

    // Control path:
    sc_signal<bool> w_req_ctrl_valid;
    sc_signal<sc_uint<AXI_ADDR_WIDTH>> wb_req_ctrl_addr;
    sc_signal<bool> w_resp_ctrl_valid;
    sc_signal<sc_uint<AXI_ADDR_WIDTH>> wb_resp_ctrl_addr;
    sc_signal<sc_uint<32>> wb_resp_ctrl_data;
    // Data path:
    sc_signal<bool> w_req_data_valid;
    sc_signal<bool> w_req_data_write;
    sc_signal<sc_uint<AXI_ADDR_WIDTH>> wb_req_data_addr;
    sc_signal<sc_uint<2>> wb_req_data_size; // 0=1bytes; 1=2bytes; 2=4bytes; 3=8bytes
    sc_signal<sc_uint<RISCV_ARCH>> wb_req_data_data;
    sc_signal<bool> w_resp_data_valid;
    sc_signal<sc_uint<AXI_ADDR_WIDTH>> wb_resp_data_addr;
    sc_signal<sc_uint<RISCV_ARCH>> wb_resp_data_data;
    sc_signal<bool> w_cache_hold;

    struct RegistersType {
        sc_signal<sc_uint<RISCV_ARCH>> timer;

    } v, r;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVER_TOP_H__
