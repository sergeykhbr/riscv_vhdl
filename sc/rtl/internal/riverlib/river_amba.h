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
#include "../ambalib/types_amba.h"
#include "types_river.h"
#include "river_cfg.h"
#include "river_top.h"
#include "l1_dma_snoop.h"

namespace debugger {

SC_MODULE(RiverAmba) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_in<sc_uint<64>> i_mtimer;                            // Read-only shadow value of memory-mapped mtimer register (see CLINT).
    sc_in<axi4_l1_in_type> i_msti;
    sc_out<axi4_l1_out_type> o_msto;
    sc_in<dport_in_type> i_dport;
    sc_out<dport_out_type> o_dport;
    sc_in<sc_uint<IRQ_TOTAL>> i_irq_pending;                // Per Hart pending interrupts pins
    sc_out<bool> o_flush_l2;                                // Flush L2 after D$ has been finished
    sc_out<bool> o_halted;                                  // CPU halted via debug interface
    sc_out<bool> o_available;                               // CPU was instantitated of stubbed
    sc_in<sc_biguint<(32 * CFG_PROGBUF_REG_TOTAL)>> i_progbuf;// progam buffer

    void comb();

    RiverAmba(sc_module_name name,
              bool async_reset,
              uint32_t hartid,
              bool fpu_ena,
              bool coherence_ena,
              bool tracer_ena);
    virtual ~RiverAmba();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;
    uint32_t hartid_;
    bool fpu_ena_;
    bool coherence_ena_;
    bool tracer_ena_;

    sc_signal<bool> req_mem_ready_i;
    sc_signal<bool> req_mem_path_o;
    sc_signal<bool> req_mem_valid_o;
    sc_signal<sc_uint<REQ_MEM_TYPE_BITS>> req_mem_type_o;
    sc_signal<sc_uint<3>> req_mem_size_o;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_mem_addr_o;
    sc_signal<sc_uint<L1CACHE_BYTES_PER_LINE>> req_mem_strob_o;
    sc_signal<sc_biguint<L1CACHE_LINE_BITS>> req_mem_data_o;
    sc_signal<sc_biguint<L1CACHE_LINE_BITS>> resp_mem_data_i;
    sc_signal<bool> resp_mem_path_i;
    sc_signal<bool> resp_mem_valid_i;
    sc_signal<bool> resp_mem_load_fault_i;
    sc_signal<bool> resp_mem_store_fault_i;
    // D$ Snoop interface
    sc_signal<bool> req_snoop_valid_i;
    sc_signal<sc_uint<SNOOP_REQ_TYPE_BITS>> req_snoop_type_i;
    sc_signal<bool> req_snoop_ready_o;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_snoop_addr_i;
    sc_signal<bool> resp_snoop_ready_i;
    sc_signal<bool> resp_snoop_valid_o;
    sc_signal<sc_biguint<L1CACHE_LINE_BITS>> resp_snoop_data_o;
    sc_signal<sc_uint<DTAG_FL_TOTAL>> resp_snoop_flags_o;
    sc_signal<bool> w_dporti_haltreq;
    sc_signal<bool> w_dporti_resumereq;
    sc_signal<bool> w_dporti_resethaltreq;
    sc_signal<bool> w_dporti_hartreset;
    sc_signal<bool> w_dporti_req_valid;
    sc_signal<sc_uint<DPortReq_Total>> wb_dporti_dtype;
    sc_signal<sc_uint<RISCV_ARCH>> wb_dporti_addr;
    sc_signal<sc_uint<RISCV_ARCH>> wb_dporti_wdata;
    sc_signal<sc_uint<3>> wb_dporti_size;
    sc_signal<bool> w_dporti_resp_ready;
    sc_signal<bool> w_dporto_req_ready;
    sc_signal<bool> w_dporto_resp_valid;
    sc_signal<bool> w_dporto_resp_error;
    sc_signal<sc_uint<RISCV_ARCH>> wb_dporto_rdata;

    RiverTop *river0;
    l1_dma_snoop<CFG_CPU_ADDR_BITS> *l1dma0;

};

}  // namespace debugger

