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
    sc_in<bool> i_e_valid;                          // Execution stage outputs are valid
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_e_pc;          // Execution stage instruction pointer
    sc_in<sc_uint<32>> i_e_instr;                   // Execution stage instruction value

    sc_in<sc_uint<5>> i_res_addr;                   // Register address to be written (0=no writing)
    sc_in<sc_uint<RISCV_ARCH>> i_res_data;          // Register value to be written
    sc_in<bool> i_memop_sign_ext;                   // Load data with sign extending (if less than 8 Bytes)
    sc_in<bool> i_memop_load;                       // Load data from memory and write to i_res_addr
    sc_in<bool> i_memop_store;                      // Store i_res_data value into memory
    sc_in<sc_uint<2>> i_memop_size;                 // Encoded memory transaction size in bytes: 0=1B; 1=2B; 2=4B; 3=8B
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_memop_addr;    // Memory access address
    sc_out<bool> o_wena;                            // Write enable signal
    sc_out<sc_uint<5>> o_waddr;                     // Output register address (0 = x0 = no write)
    sc_out<sc_uint<RISCV_ARCH>> o_wdata;            // Register value

    // Memory interface:
    sc_in<bool> i_mem_req_ready;                    // Data cache is ready to accept request
    sc_out<bool> o_mem_valid;                       // Memory request is valid
    sc_out<bool> o_mem_write;                       // Memory write request
    sc_out<sc_uint<2>> o_mem_sz;                    // Encoded data size in bytes: 0=1B; 1=2B; 2=4B; 3=8B
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_mem_addr;     // Data path requested address
    sc_out<sc_uint<BUS_DATA_WIDTH>> o_mem_data;     // Data path requested data (write transaction)
    sc_in<bool> i_mem_data_valid;                   // Data path memory response is valid
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_mem_data_addr; // Data path memory response address
    sc_in<sc_uint<BUS_DATA_WIDTH>> i_mem_data;      // Data path memory response value
    sc_out<bool> o_mem_resp_ready;                  // Pipeline is ready to accept memory operation response

    sc_out<bool> o_hold;                            // Hold pipeline by data cache wait state reason
    sc_out<bool> o_valid;                           // Output is valid
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_pc;           // Valid instruction pointer
    sc_out<sc_uint<32>> o_instr;                    // Valid instruction value

    void comb();
    void registers();

    SC_HAS_PROCESS(MemAccess);

    MemAccess(sc_module_name name_, sc_trace_file *vcd=0);

private:
    struct RegistersType {
        sc_signal<bool> valid;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> pc;
        sc_signal<sc_uint<32>> instr;

        sc_signal<bool> wena;
        sc_signal<sc_uint<5>> waddr;
        sc_signal<bool> sign_ext;
        sc_signal<sc_uint<2>> size;
        sc_signal<sc_uint<RISCV_ARCH>> wdata;
        sc_signal<bool> wait_req;
        sc_signal<bool> wait_req_write;
        sc_signal<sc_uint<2>> wait_req_sz;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> wait_req_addr;
        sc_signal<sc_uint<RISCV_ARCH>> wait_req_wdata;
        sc_signal<bool> wait_resp;
    } v, r;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_EXECUTE_H__
