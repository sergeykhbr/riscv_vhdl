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
#include "river_cfg.h"
#include "core/proc.h"
#include "cache/cache_top.h"

namespace debugger {

SC_MODULE(RiverTop) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<sc_uint<64>> i_mtimer;                            // Read-only shadow value of memory-mapped mtimer register (see CLINT).
    // Memory interface:
    sc_in<bool> i_req_mem_ready;                            // System Bus is ready to accept memory operation request
    sc_out<bool> o_req_mem_path;                            // 0=ctrl; 1=data path
    sc_out<bool> o_req_mem_valid;                           // AXI memory request is valid
    sc_out<sc_uint<REQ_MEM_TYPE_BITS>> o_req_mem_type;      // AXI memory request type
    sc_out<sc_uint<3>> o_req_mem_size;                      // request size: 0=1 B;...; 7=128 B
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_req_mem_addr;      // AXI memory request address
    sc_out<sc_uint<L1CACHE_BYTES_PER_LINE>> o_req_mem_strob;// Writing strob. 1 bit per Byte (uncached only)
    sc_out<sc_biguint<L1CACHE_LINE_BITS>> o_req_mem_data;   // Writing data
    sc_in<bool> i_resp_mem_valid;                           // AXI response is valid
    sc_in<bool> i_resp_mem_path;                            // 0=ctrl; 1=data path
    sc_in<sc_biguint<L1CACHE_LINE_BITS>> i_resp_mem_data;   // Read data
    sc_in<bool> i_resp_mem_load_fault;                      // data load error
    sc_in<bool> i_resp_mem_store_fault;                     // data store error
    // $D Snoop interface:
    sc_in<bool> i_req_snoop_valid;
    sc_in<sc_uint<SNOOP_REQ_TYPE_BITS>> i_req_snoop_type;
    sc_out<bool> o_req_snoop_ready;
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_req_snoop_addr;
    sc_in<bool> i_resp_snoop_ready;
    sc_out<bool> o_resp_snoop_valid;
    sc_out<sc_biguint<L1CACHE_LINE_BITS>> o_resp_snoop_data;
    sc_out<sc_uint<DTAG_FL_TOTAL>> o_resp_snoop_flags;
    sc_out<bool> o_flush_l2;                                // Flush L2 after D$ has been finished
    // Interrupt lines:
    sc_in<sc_uint<IRQ_TOTAL>> i_irq_pending;                // Per Hart pending interrupts pins
    // Debug interface:
    sc_in<bool> i_haltreq;                                  // DMI: halt request from debug unit
    sc_in<bool> i_resumereq;                                // DMI: resume request from debug unit
    sc_in<bool> i_dport_req_valid;                          // Debug access from DSU is valid
    sc_in<sc_uint<DPortReq_Total>> i_dport_type;            // Debug access type
    sc_in<sc_uint<RISCV_ARCH>> i_dport_addr;                // dport address
    sc_in<sc_uint<RISCV_ARCH>> i_dport_wdata;               // Write value
    sc_in<sc_uint<3>> i_dport_size;                         // reg/mem access size:0=1B;...,4=128B;
    sc_out<bool> o_dport_req_ready;
    sc_in<bool> i_dport_resp_ready;                         // ready to accepd response
    sc_out<bool> o_dport_resp_valid;                        // Response is valid
    sc_out<bool> o_dport_resp_error;                        // Something wrong during command execution
    sc_out<sc_uint<RISCV_ARCH>> o_dport_rdata;              // Response value
    sc_in<sc_biguint<(32 * CFG_PROGBUF_REG_TOTAL)>> i_progbuf;// progam buffer
    sc_out<bool> o_halted;                                  // CPU halted via debug interface

    void comb();

    SC_HAS_PROCESS(RiverTop);

    RiverTop(sc_module_name name,
             bool async_reset,
             uint32_t hartid,
             bool fpu_ena,
             bool coherence_ena,
             bool tracer_ena,
             uint32_t ilog2_nways,
             uint32_t ilog2_lines_per_way,
             uint32_t dlog2_nways,
             uint32_t dlog2_lines_per_way);
    virtual ~RiverTop();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;
    uint32_t hartid_;
    bool fpu_ena_;
    bool coherence_ena_;
    bool tracer_ena_;
    uint32_t ilog2_nways_;
    uint32_t ilog2_lines_per_way_;
    uint32_t dlog2_nways_;
    uint32_t dlog2_lines_per_way_;

    // Control path:
    sc_signal<bool> w_req_ctrl_ready;
    sc_signal<bool> w_req_ctrl_valid;
    sc_signal<sc_uint<RISCV_ARCH>> wb_req_ctrl_addr;
    sc_signal<bool> w_resp_ctrl_valid;
    sc_signal<sc_uint<RISCV_ARCH>> wb_resp_ctrl_addr;
    sc_signal<sc_uint<64>> wb_resp_ctrl_data;
    sc_signal<bool> w_resp_ctrl_load_fault;
    sc_signal<bool> w_resp_ctrl_ready;
    // Data path:
    sc_signal<bool> w_req_data_ready;
    sc_signal<bool> w_req_data_valid;
    sc_signal<sc_uint<MemopType_Total>> wb_req_data_type;
    sc_signal<sc_uint<RISCV_ARCH>> wb_req_data_addr;
    sc_signal<sc_uint<64>> wb_req_data_wdata;
    sc_signal<sc_uint<8>> wb_req_data_wstrb;
    sc_signal<sc_uint<2>> wb_req_data_size;
    sc_signal<bool> w_resp_data_valid;
    sc_signal<sc_uint<RISCV_ARCH>> wb_resp_data_addr;
    sc_signal<sc_uint<64>> wb_resp_data_data;
    sc_signal<bool> w_resp_data_load_fault;
    sc_signal<bool> w_resp_data_store_fault;
    sc_signal<bool> w_resp_data_ready;
    sc_signal<bool> w_pmp_ena;
    sc_signal<bool> w_pmp_we;
    sc_signal<sc_uint<CFG_PMP_TBL_WIDTH>> wb_pmp_region;
    sc_signal<sc_uint<RISCV_ARCH>> wb_pmp_start_addr;
    sc_signal<sc_uint<RISCV_ARCH>> wb_pmp_end_addr;
    sc_signal<sc_uint<CFG_PMP_FL_TOTAL>> wb_pmp_flags;
    sc_signal<bool> w_flushi_valid;
    sc_signal<sc_uint<RISCV_ARCH>> wb_flushi_addr;
    sc_signal<bool> w_flushd_valid;
    sc_signal<sc_uint<RISCV_ARCH>> wb_flushd_addr;
    sc_signal<bool> w_flushd_end;

    Processor *proc0;
    CacheTop *cache0;

};

}  // namespace debugger

