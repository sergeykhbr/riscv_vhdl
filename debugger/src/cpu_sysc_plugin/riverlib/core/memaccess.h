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

    void main();
    void comb();
    void qproc();
    void registers();

    SC_HAS_PROCESS(MemAccess);

    MemAccess(sc_module_name name_, bool async_reset);

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

private:
    static const unsigned State_Idle = 0;
    static const unsigned State_WaitReqAccept = 1;
    static const unsigned State_WaitResponse = 2;
    static const unsigned State_RegForward = 3;

    struct RegistersType {
        sc_signal<sc_uint<2>> state;
        sc_signal<bool> memop_r;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> memop_addr;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> pc;
        sc_signal<sc_uint<32>> instr;
        sc_signal<sc_uint<6>> res_addr;
        sc_signal<sc_uint<RISCV_ARCH>> res_data;
        sc_signal<sc_uint<BUS_DATA_WIDTH>> memop_wdata;
        sc_signal<sc_uint<BUS_DATA_BYTES>> memop_wstrb;
        sc_signal<bool> memop_sign_ext;
        sc_signal<sc_uint<2>> memop_size;
        sc_signal<bool> wena;
    } v, r;

    void R_RESET(RegistersType &iv) {
        iv.state = State_Idle;
        iv.memop_r = 0;
        iv.memop_addr = 0;
        iv.pc = 0;
        iv.instr = 0;
        iv.res_addr = 0;
        iv.res_data = 0;
        iv.memop_wdata = 0;
        iv.memop_wstrb = 0;
        iv.memop_sign_ext = 0;
        iv.memop_size = 0;
        iv.wena = 0;
    }

    static const int QUEUE_WIDTH = RISCV_ARCH + 6 + 32 + BUS_ADDR_WIDTH
                                 + 2 + 1 + 1 + 1 + BUS_DATA_WIDTH
                                 + BUS_DATA_WIDTH + BUS_DATA_BYTES;
    static const int QUEUE_DEPTH = 1;

    struct QueueRegisterType {
        sc_signal<sc_uint<2>> wcnt;
        sc_signal<sc_biguint<QUEUE_WIDTH>> mem[QUEUE_DEPTH];
    } qv, qr;

    sc_signal<bool> queue_we;
    sc_signal<bool> queue_re;
    sc_signal<sc_biguint<QUEUE_WIDTH>> queue_data_i;
    sc_signal<sc_biguint<QUEUE_WIDTH>> queue_data_o;
    sc_signal<bool> queue_nempty;

    bool async_reset_;
};

}  // namespace debugger

#endif  // __DEBUGGER_RIVERLIB_EXECUTE_H__
