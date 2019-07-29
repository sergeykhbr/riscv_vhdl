/*
 *  Copyright 2019 Sergey Khabarov, sergeykhbr@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef __DEBUGGER_RIVERLIB_MEMSTAGE_H__
#define __DEBUGGER_RIVERLIB_MEMSTAGE_H__

#include <systemc.h>
#include "../river_cfg.h"

namespace debugger {

#define MEM_V2

SC_MODULE(MemAccess) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<bool> i_e_valid;                          // Execution stage outputs are valid
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_e_pc;          // Execution stage instruction pointer
    sc_in<sc_uint<32>> i_e_instr;                   // Execution stage instruction value

    sc_in<sc_uint<6>> i_res_addr;                   // Register address to be written (0=no writing)
    sc_in<sc_uint<RISCV_ARCH>> i_res_data;          // Register value to be written
    sc_in<bool> i_memop_sign_ext;                   // Load data with sign extending (if less than 8 Bytes)
    sc_in<bool> i_memop_load;                       // Load data from memory and write to i_res_addr
    sc_in<bool> i_memop_store;                      // Store i_res_data value into memory
    sc_in<sc_uint<2>> i_memop_size;                 // Encoded memory transaction size in bytes: 0=1B; 1=2B; 2=4B; 3=8B
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_memop_addr;    // Memory access address
    sc_out<bool> o_wena;                            // Write enable signal
    sc_out<sc_uint<6>> o_waddr;                     // Output register address (0 = x0 = no write)
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

    MemAccess(sc_module_name name_, bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

private:
    struct RegistersType {
#ifdef MEM_V2
        sc_signal<bool> requested;
        sc_signal<bool> req_valid;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> req_pc;
        sc_signal<sc_uint<32>> req_instr;
        sc_signal<sc_uint<6>> req_res_addr;
        sc_signal<sc_uint<RISCV_ARCH>> req_res_data;
        sc_signal<bool> req_memop_sign_ext;
        sc_signal<bool> req_memop;
        sc_signal<bool> req_memop_store;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> req_memop_addr;
        sc_signal<sc_uint<2>> req_memop_size;
        sc_signal<bool> req_wena;
#else
        sc_signal<bool> valid;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> pc;
        sc_signal<sc_uint<32>> instr;

        sc_signal<bool> wena;
        sc_signal<sc_uint<6>> waddr;
        sc_signal<bool> sign_ext;
        sc_signal<sc_uint<2>> size;
        sc_signal<sc_uint<RISCV_ARCH>> wdata;
        sc_signal<bool> wait_req;
        sc_signal<bool> wait_req_write;
        sc_signal<sc_uint<2>> wait_req_sz;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> wait_req_addr;
        sc_signal<sc_uint<RISCV_ARCH>> wait_req_wdata;
        sc_signal<bool> wait_resp;
#endif
    } v, r;

    void R_RESET(RegistersType &iv) {
#ifdef MEM_V2
        iv.requested = 0;
        iv.req_valid = 0;
        iv.req_pc = 0;
        iv.req_instr = 0;
        iv.req_res_addr = 0;
        iv.req_res_data = 0;
        iv.req_memop_sign_ext = 0;
        iv.req_memop = 0;
        iv.req_memop_store = 0;
        iv.req_memop_addr = 0;
        iv.req_memop_size = 0;
        iv.req_wena = 0;
#else
        iv.valid = 0;
        iv.pc = 0;
        iv.instr = 0;
        iv.wena = 0;
        iv.waddr = 0;
        iv.sign_ext = 0;
        iv.size = 0;
        iv.wdata = 0;
        iv.wait_req = 0;
        iv.wait_req_write = 0;
        iv.wait_req_sz = 0;
        iv.wait_req_addr = 0;
        iv.wait_req_wdata = 0;
        iv.wait_resp = 0;
#endif
    }

    bool async_reset_;
};


}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_EXECUTE_H__
