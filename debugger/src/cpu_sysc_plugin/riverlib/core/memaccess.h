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
#include "queue.h"

namespace debugger {

SC_MODULE(MemAccess) {
    sc_in<bool> i_clk;
    sc_in<bool> i_nrst;
    sc_in<bool> i_e_valid;                          // Execution stage outputs are valid
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_e_pc;          // Execution stage instruction pointer
    sc_in<sc_uint<32>> i_e_instr;                   // Execution stage instruction value

    sc_in<sc_uint<6>> i_memop_waddr;                // Register address to be written (0=no writing)
    sc_in<sc_uint<4>> i_memop_wtag;                
    sc_in<sc_uint<RISCV_ARCH>> i_memop_wdata;       // Register value to be written
    sc_in<bool> i_memop_sign_ext;                   // Load data with sign extending (if less than 8 Bytes)
    sc_in<bool> i_memop_load;                       // Load data from memory and write to i_res_addr
    sc_in<bool> i_memop_store;                      // Store i_res_data value into memory
    sc_in<sc_uint<2>> i_memop_size;                 // Encoded memory transaction size in bytes: 0=1B; 1=2B; 2=4B; 3=8B
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_memop_addr;    // Memory access address
    sc_out<bool> o_memop_ready;                     // Ready to accept memop request
    sc_out<bool> o_wena;                            // Write enable signal
    sc_out<sc_uint<6>> o_waddr;                     // Output register address (0 = x0 = no write)
    sc_out<sc_uint<RISCV_ARCH>> o_wdata;            // Register value

    // Memory interface:
    sc_in<bool> i_mem_req_ready;                    // Data cache is ready to accept read/write request
    sc_out<bool> o_mem_valid;                       // Memory request is valid
    sc_out<bool> o_mem_write;                       // Memory write request
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_mem_addr;     // Data path requested address
    sc_out<sc_uint<BUS_DATA_WIDTH>> o_mem_wdata;    // Data path requested data (write transaction)
    sc_out<sc_uint<BUS_DATA_BYTES>> o_mem_wstrb;    // 8-bytes aligned strobs
    sc_in<bool> i_mem_data_valid;                   // Data path memory response is valid
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_mem_data_addr; // Data path memory response address
    sc_in<sc_uint<BUS_DATA_WIDTH>> i_mem_data;      // Data path memory response value
    sc_out<bool> o_mem_resp_ready;                  // Pipeline is ready to accept memory operation response

    sc_out<bool> o_hold;                            // memory access hold-on
    sc_out<bool> o_valid;                           // Output is valid
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_pc;           // Valid instruction pointer
    sc_out<sc_uint<32>> o_instr;                    // Valid instruction value
    sc_out<bool> o_wb_memop;                        // memory operation write back (for tracer only)

    void comb();
    void registers();

    SC_HAS_PROCESS(MemAccess);

    MemAccess(sc_module_name name_, bool async_reset);
    virtual ~MemAccess();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

private:
    static const unsigned State_Idle = 0;
    static const unsigned State_WaitReqAccept = 1;
    static const unsigned State_WaitResponse = 2;
    static const unsigned State_Hold = 3;

    struct RegistersType {
        sc_signal<sc_uint<2>> state;
        sc_signal<bool> memop_w;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> memop_addr;
        sc_signal<sc_uint<BUS_DATA_WIDTH>> memop_wdata;
        sc_signal<sc_uint<BUS_DATA_BYTES>> memop_wstrb;
        sc_signal<bool> memop_sign_ext;
        sc_signal<sc_uint<2>> memop_size;

        sc_signal<sc_uint<BUS_ADDR_WIDTH>> memop_res_pc;
        sc_signal<sc_uint<32>> memop_res_instr;
        sc_signal<sc_uint<6>> memop_res_addr;
        sc_signal<sc_uint<RISCV_ARCH>> memop_res_data;
        sc_signal<bool> memop_res_wena;

        sc_signal<bool> reg_wb_valid;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> reg_res_pc;
        sc_signal<sc_uint<32>> reg_res_instr;
        sc_signal<sc_uint<6>> reg_res_addr;
        sc_signal<sc_uint<RISCV_ARCH>> reg_res_data;
        sc_signal<bool> reg_res_wena;

        sc_signal<sc_uint<BUS_ADDR_WIDTH>> hold_res_pc;
        sc_signal<sc_uint<32>> hold_res_instr;
        sc_signal<sc_uint<6>> hold_res_waddr;
        sc_signal<sc_uint<RISCV_ARCH>> hold_res_wdata;
        sc_signal<bool> hold_res_wena;
    } v, r;

    void R_RESET(RegistersType &iv) {
        iv.state = State_Idle;
        iv.memop_w = 0;
        iv.memop_addr = 0;
        iv.memop_wdata = 0;
        iv.memop_wstrb = 0;
        iv.memop_sign_ext = 0;
        iv.memop_size = 0;
        iv.memop_res_pc = 0;
        iv.memop_res_instr = 0;
        iv.memop_res_addr = 0;
        iv.memop_res_data = 0;
        iv.memop_res_wena = 0;
        iv.reg_wb_valid = 0;
        iv.reg_res_pc = 0;
        iv.reg_res_instr = 0;
        iv.reg_res_addr = 0;
        iv.reg_res_data = 0;
        iv.reg_res_wena = 0;
        iv.hold_res_pc = 0;
        iv.hold_res_instr = 0;
        iv.hold_res_waddr = 0;
        iv.hold_res_wdata = 0;
        iv.hold_res_wena = 0;
    }

    static const int QUEUE_WIDTH = 4
                                 + BUS_DATA_WIDTH
                                 + BUS_DATA_BYTES 
                                 + RISCV_ARCH
                                 + 6
                                 + 32
                                 + BUS_ADDR_WIDTH
                                 + 2
                                 + 1
                                 + 1
                                 + BUS_ADDR_WIDTH;

    Queue<2, QUEUE_WIDTH> *queue0;

    sc_signal<bool> queue_we;
    sc_signal<bool> queue_re;
    sc_signal<sc_biguint<QUEUE_WIDTH>> queue_data_i;
    sc_signal<sc_biguint<QUEUE_WIDTH>> queue_data_o;
    sc_signal<bool> queue_nempty;
    sc_signal<bool> queue_full;

    bool async_reset_;
};

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_EXECUTE_H__
