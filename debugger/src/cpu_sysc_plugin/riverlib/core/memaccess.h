/**
 * @file
 * @copyright  Copyright 2016 GNSS Sensor Ltd. All right reserved.
 * @author     Sergey Khabarov - sergeykhbr@gmail.com
 * @brief      CPU Memory Access stage.
 */

#ifndef __DEBUGGER_RIVERLIB_MEMSTAGE_H__
#define __DEBUGGER_RIVERLIB_MEMSTAGE_H__

#include <systemc.h>
#include "../river_cfg.h"

namespace debugger {

SC_MODULE(MemAccess) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<bool> i_e_valid;
    sc_in<sc_uint<AXI_ADDR_WIDTH>> i_e_pc;
    sc_in<sc_uint<32>> i_e_instr;

    sc_in<sc_uint<5>> i_res_addr;
    sc_in<sc_uint<RISCV_ARCH>> i_res_data;
    sc_in<bool> i_memop_load;
    sc_in<bool> i_memop_store;
    sc_in<sc_uint<2>> i_memop_size;
    sc_in<sc_uint<AXI_ADDR_WIDTH>> i_memop_addr;    // Memory access address
    sc_out<bool> o_wena;
    sc_out<sc_uint<5>> o_waddr;
    sc_out<sc_uint<RISCV_ARCH>> o_wdata;

    // Memory interface:
    sc_out<bool> o_mem_valid;
    sc_out<bool> o_mem_write;
    sc_out<sc_uint<2>> o_mem_sz;
    sc_out<sc_uint<AXI_ADDR_WIDTH>> o_mem_addr;
    sc_out<sc_uint<AXI_DATA_WIDTH>> o_mem_data;
    sc_in<bool> i_mem_data_valid;
    sc_in<sc_uint<AXI_ADDR_WIDTH>> i_mem_data_addr;
    sc_in<sc_uint<AXI_DATA_WIDTH>> i_mem_data;

    sc_out<bool> o_valid;
    sc_out<sc_uint<AXI_ADDR_WIDTH>> o_pc;
    sc_out<sc_uint<32>> o_instr;


    void comb();
    void registers();

    SC_HAS_PROCESS(MemAccess);

    MemAccess(sc_module_name name_, sc_trace_file *vcd=0);

private:
    struct RegistersType {
        sc_signal<bool> valid;
        sc_signal<sc_uint<AXI_ADDR_WIDTH>> pc;
        sc_signal<sc_uint<32>> instr;

        sc_signal<bool> wait_resp;
        sc_signal<bool> wena;
        sc_signal<sc_uint<5>> waddr;
        sc_signal<sc_uint<RISCV_ARCH>> wdata;
    } v, r;

    bool w_mem_valid;
    bool w_mem_write;
    sc_uint<2> wb_mem_sz;
    sc_uint<AXI_ADDR_WIDTH> wb_mem_addr;
    sc_uint<RISCV_ARCH> wb_mem_wdata;
    sc_uint<RISCV_ARCH> wb_res_wdata;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_EXECUTE_H__
