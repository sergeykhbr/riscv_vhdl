// 
//  Copyright 2022 Sergey Khabarov, sergeykhbr@gmail.com
// 
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
// 
//      http://www.apache.org/licenses/LICENSE-2.0
// 
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
// 
#pragma once

#include <systemc.h>
#include "../river_cfg.h"
#include "queue.h"

namespace debugger {

SC_MODULE(MemAccess) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<sc_uint<RISCV_ARCH>> i_e_pc;                      // Execution stage instruction pointer
    sc_in<sc_uint<32>> i_e_instr;                           // Execution stage instruction value
    sc_in<bool> i_flushd_valid;
    sc_in<sc_uint<RISCV_ARCH>> i_flushd_addr;
    sc_out<bool> o_flushd;
    sc_in<bool> i_mmu_ena;                                  // MMU enabled
    sc_in<bool> i_mmu_sv39;                                 // MMU sv39 mode is enabled
    sc_in<bool> i_mmu_sv48;                                 // MMU sv48 mode is enabled
    sc_out<bool> o_mmu_ena;                                 // Delayed MMU enabled
    sc_out<bool> o_mmu_sv39;                                // Delayed MMU sv39 mode is enabled
    sc_out<bool> o_mmu_sv48;                                // Delayed MMU sv48 mode is enabled
    sc_in<sc_uint<6>> i_reg_waddr;                          // Register address to be written (0=no writing)
    sc_in<sc_uint<CFG_REG_TAG_WIDTH>> i_reg_wtag;           // Register tag for writeback operation
    sc_in<bool> i_memop_valid;                              // Memory request is valid
    sc_in<bool> i_memop_debug;                              // Memory debug request
    sc_in<sc_uint<RISCV_ARCH>> i_memop_wdata;               // Register value to be written
    sc_in<bool> i_memop_sign_ext;                           // Load data with sign extending (if less than 8 Bytes)
    sc_in<sc_uint<MemopType_Total>> i_memop_type;           // [0] 1=store;0=Load data from memory and write to i_res_addr
    sc_in<sc_uint<2>> i_memop_size;                         // Encoded memory transaction size in bytes: 0=1B; 1=2B; 2=4B; 3=8B
    sc_in<sc_uint<RISCV_ARCH>> i_memop_addr;                // Memory access address
    sc_out<bool> o_memop_ready;                             // Ready to accept memop request
    sc_out<bool> o_wb_wena;                                 // Write enable signal
    sc_out<sc_uint<6>> o_wb_waddr;                          // Output register address (0 = x0 = no write)
    sc_out<sc_uint<RISCV_ARCH>> o_wb_wdata;                 // Register value
    sc_out<sc_uint<CFG_REG_TAG_WIDTH>> o_wb_wtag;
    sc_in<bool> i_wb_ready;
    // Memory interface:
    sc_in<bool> i_mem_req_ready;                            // Data cache is ready to accept read/write request
    sc_out<bool> o_mem_valid;                               // Memory request is valid
    sc_out<sc_uint<MemopType_Total>> o_mem_type;            // Memory operation type
    sc_out<sc_uint<RISCV_ARCH>> o_mem_addr;                 // Data path requested address
    sc_out<sc_uint<64>> o_mem_wdata;                        // Data path requested data (write transaction)
    sc_out<sc_uint<8>> o_mem_wstrb;                         // 8-bytes aligned strobs
    sc_out<sc_uint<2>> o_mem_size;                          // 1,2,4 or 8-bytes operation for uncached access
    sc_in<bool> i_mem_data_valid;                           // Data path memory response is valid
    sc_in<sc_uint<RISCV_ARCH>> i_mem_data_addr;             // Data path memory response address
    sc_in<sc_uint<64>> i_mem_data;                          // Data path memory response value
    sc_out<bool> o_mem_resp_ready;                          // Pipeline is ready to accept memory operation response
    sc_out<sc_uint<RISCV_ARCH>> o_pc;                       // executed memory/flush request only
    sc_out<bool> o_valid;                                   // memory/flush operation completed
    sc_out<bool> o_idle;                                    // All memory operation completed
    sc_out<bool> o_debug_valid;                             // Debug request processed, response is valid

    void comb();
    void registers();

    SC_HAS_PROCESS(MemAccess);

    MemAccess(sc_module_name name,
              bool async_reset);
    virtual ~MemAccess();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    static const uint8_t State_Idle = 0;
    static const uint8_t State_WaitReqAccept = 1;
    static const uint8_t State_WaitResponse = 2;
    static const uint8_t State_Hold = 3;
    static const int QUEUE_WIDTH = (1  // memop_debug
            + 1  // i_flushd_valid
            + 1  // i_mmu_ena
            + 1  // i_mmu_sv39
            + 1  // i_mmu_sv48
            + CFG_REG_TAG_WIDTH  // vb_res_wtag
            + 64  // vb_mem_wdata
            + 8  // vb_mem_wstrb
            + RISCV_ARCH  // vb_res_data
            + 6  // vb_res_addr
            + 32  // vb_e_instr
            + RISCV_ARCH  // vb_e_pc
            + 2  // vb_mem_sz
            + 1  // v_mem_sign_ext
            + MemopType_Total  // vb_mem_type
            + RISCV_ARCH  // vb_mem_addr
    );

    struct MemAccess_registers {
        sc_signal<sc_uint<2>> state;
        sc_signal<bool> mmu_ena;
        sc_signal<bool> mmu_sv39;
        sc_signal<bool> mmu_sv48;
        sc_signal<sc_uint<MemopType_Total>> memop_type;
        sc_signal<sc_uint<RISCV_ARCH>> memop_addr;
        sc_signal<sc_uint<64>> memop_wdata;
        sc_signal<sc_uint<8>> memop_wstrb;
        sc_signal<bool> memop_sign_ext;
        sc_signal<sc_uint<2>> memop_size;
        sc_signal<bool> memop_debug;
        sc_signal<sc_uint<RISCV_ARCH>> memop_res_pc;
        sc_signal<sc_uint<32>> memop_res_instr;
        sc_signal<sc_uint<6>> memop_res_addr;
        sc_signal<sc_uint<CFG_REG_TAG_WIDTH>> memop_res_wtag;
        sc_signal<sc_uint<RISCV_ARCH>> memop_res_data;
        sc_signal<bool> memop_res_wena;
        sc_signal<sc_uint<RISCV_ARCH>> hold_rdata;
        sc_signal<sc_uint<RISCV_ARCH>> pc;
        sc_signal<bool> valid;
    } v, r;

    void MemAccess_r_reset(MemAccess_registers &iv) {
        iv.state = State_Idle;
        iv.mmu_ena = 0;
        iv.mmu_sv39 = 0;
        iv.mmu_sv48 = 0;
        iv.memop_type = 0;
        iv.memop_addr = 0ull;
        iv.memop_wdata = 0ull;
        iv.memop_wstrb = 0;
        iv.memop_sign_ext = 0;
        iv.memop_size = 0;
        iv.memop_debug = 0;
        iv.memop_res_pc = 0ull;
        iv.memop_res_instr = 0;
        iv.memop_res_addr = 0;
        iv.memop_res_wtag = 0;
        iv.memop_res_data = 0ull;
        iv.memop_res_wena = 0;
        iv.hold_rdata = 0ull;
        iv.pc = 0ull;
        iv.valid = 0;
    }

    sc_signal<bool> queue_we;
    sc_signal<bool> queue_re;
    sc_signal<sc_biguint<QUEUE_WIDTH>> queue_data_i;
    sc_signal<sc_biguint<QUEUE_WIDTH>> queue_data_o;
    sc_signal<bool> queue_nempty;
    sc_signal<bool> queue_full;

    Queue<CFG_MEMACCESS_QUEUE_DEPTH, QUEUE_WIDTH> *queue0;

};

}  // namespace debugger

