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
#include "../ambalib/types_amba.h"
#include "types_river.h"
#include "river_top.h"

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
    void registers();

    SC_HAS_PROCESS(RiverAmba);

    RiverAmba(sc_module_name name,
              bool async_reset,
              uint32_t hartid,
              bool fpu_ena,
              bool coherence_ena,
              bool tracer_ena,
              uint32_t ilog2_nways,
              uint32_t ilog2_lines_per_way,
              uint32_t dlog2_nways,
              uint32_t dlog2_lines_per_way);
    virtual ~RiverAmba();

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

    static const uint8_t state_idle = 0;
    static const uint8_t state_ar = 1;
    static const uint8_t state_r = 2;
    static const uint8_t state_aw = 3;
    static const uint8_t state_w = 4;
    static const uint8_t state_b = 5;
    static const uint8_t snoop_idle = 0;
    static const uint8_t snoop_ac_wait_accept = 1;
    static const uint8_t snoop_cr = 2;
    static const uint8_t snoop_cr_wait_accept = 3;
    static const uint8_t snoop_cd = 4;
    static const uint8_t snoop_cd_wait_accept = 5;

    sc_uint<4> reqtype2arsnoop(sc_uint<REQ_MEM_TYPE_BITS> reqtype);
    sc_uint<4> reqtype2awsnoop(sc_uint<REQ_MEM_TYPE_BITS> reqtype);

    struct RiverAmba_registers {
        sc_signal<sc_uint<3>> state;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_addr;
        sc_signal<bool> req_path;
        sc_signal<sc_uint<4>> req_cached;
        sc_signal<sc_biguint<L1CACHE_LINE_BITS>> req_wdata;
        sc_signal<sc_uint<L1CACHE_BYTES_PER_LINE>> req_wstrb;
        sc_signal<sc_uint<3>> req_size;
        sc_signal<sc_uint<3>> req_prot;
        sc_signal<sc_uint<4>> req_ar_snoop;
        sc_signal<sc_uint<3>> req_aw_snoop;
        sc_signal<sc_uint<3>> snoop_state;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> ac_addr;
        sc_signal<sc_uint<4>> ac_snoop;                     // Table C3-19
        sc_signal<sc_uint<5>> cr_resp;
        sc_signal<sc_uint<SNOOP_REQ_TYPE_BITS>> req_snoop_type;
        sc_signal<sc_biguint<L1CACHE_LINE_BITS>> resp_snoop_data;
        sc_signal<bool> cache_access;
    } v, r;

    void RiverAmba_r_reset(RiverAmba_registers &iv) {
        iv.state = 0;
        iv.req_addr = state_idle;
        iv.req_path = 0;
        iv.req_cached = 0;
        iv.req_wdata = 0ull;
        iv.req_wstrb = 0;
        iv.req_size = 0;
        iv.req_prot = 0;
        iv.req_ar_snoop = 0;
        iv.req_aw_snoop = 0;
        iv.snoop_state = snoop_idle;
        iv.ac_addr = 0ull;
        iv.ac_snoop = 0;
        iv.cr_resp = 0;
        iv.req_snoop_type = 0;
        iv.resp_snoop_data = 0ull;
        iv.cache_access = 0;
    }

    sc_signal<bool> req_mem_ready_i;
    sc_signal<bool> req_mem_path_o;
    sc_signal<bool> req_mem_valid_o;
    sc_signal<sc_uint<REQ_MEM_TYPE_BITS>> req_mem_type_o;
    sc_signal<sc_uint<3>> req_mem_size_o;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_mem_addr_o;
    sc_signal<sc_uint<L1CACHE_BYTES_PER_LINE>> req_mem_strob_o;
    sc_signal<sc_biguint<L1CACHE_LINE_BITS>> req_mem_data_o;
    sc_signal<sc_biguint<L1CACHE_LINE_BITS>> resp_mem_data_i;
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

};

}  // namespace debugger

