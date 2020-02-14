/*
 *  Copyright 2018 Sergey Khabarov, sergeykhbr@gmail.com
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

#ifndef __DEBUGGER_RIVER_AMBA_H__
#define __DEBUGGER_RIVER_AMBA_H__

#include "river_cfg.h"
#include "types_river.h"
#include "river_top.h"
#include <systemc.h>

namespace debugger {

SC_MODULE(RiverAmba) {
    sc_in<bool> i_clk;                                  // CPU clock
    sc_in<bool> i_nrst;                                 // Reset: active LOW
    sc_in<axi4_l1_in_type> i_msti;
    sc_out<axi4_l1_out_type> o_msto;
    /** Interrupt line from external interrupts controller (PLIC). */
    sc_in<bool> i_ext_irq;
    sc_out<sc_uint<64>> o_time;                         // Clock/Step counter depending attribute "GenerateRef"
    sc_out<sc_uint<64>> o_exec_cnt;
    // Debug interface
    sc_in<bool> i_dport_valid;                          // Debug access from DSU is valid
    sc_in<bool> i_dport_write;                          // Write command flag
    sc_in<sc_uint<2>> i_dport_region;                   // Registers region ID: 0=CSR; 1=IREGS; 2=Control
    sc_in<sc_uint<12>> i_dport_addr;                    // Register idx
    sc_in<sc_uint<RISCV_ARCH>> i_dport_wdata;           // Write value
    sc_out<bool> o_dport_ready;                         // Response is ready
    sc_out<sc_uint<RISCV_ARCH>> o_dport_rdata;          // Response value
    sc_out<bool> o_halted;                              // CPU halted via debug interface

    void comb();
    void registers();

    SC_HAS_PROCESS(RiverAmba);

    RiverAmba(sc_module_name name_,
             uint32_t hartid,
             bool async_reset,
             bool fpu_ena,
             bool tracer_ena);
    virtual ~RiverAmba();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    RiverTop *river0;

    static const unsigned ARCACHE_DEVICE_NON_BUFFERABLE = 0x0;  // 4'b0000
    static const unsigned ARCACHE_WRBACK_READ_ALLOCATE  = 0xF;  // 4'b1111

    static const unsigned AWCACHE_DEVICE_NON_BUFFERABLE = 0x0;  // 4'b0000
    static const unsigned AWCACHE_WRBACK_WRITE_ALLOCATE = 0xF;  // 4'b1111

    sc_signal<bool> req_mem_ready_i;
    sc_signal<bool> req_mem_path_o;
    sc_signal<bool> req_mem_valid_o;
    sc_signal<bool> req_mem_write_o;
    sc_signal<bool> req_mem_cached_o;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_mem_addr_o;
    sc_signal<sc_uint<L1CACHE_BYTES_PER_LINE>> req_mem_strob_o;
    sc_signal<sc_biguint<L1CACHE_LINE_BITS>> req_mem_data_o;
    sc_signal<sc_biguint<L1CACHE_LINE_BITS>> resp_mem_data_i;
    sc_signal<bool> resp_mem_valid_i;
    sc_signal<bool> resp_mem_load_fault_i;
    sc_signal<bool> resp_mem_store_fault_i;
    // D$ Snoop interface
    sc_signal<bool> req_snoop_valid_i;
    sc_signal<bool> req_snoop_getdata_i;      // 0=check availability; 1=read line
    sc_signal<bool> req_snoop_ready_o;
    sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_snoop_addr_i;
    sc_signal<bool> resp_snoop_ready_i;
    sc_signal<bool> resp_snoop_valid_o;
    sc_signal<sc_biguint<L1CACHE_LINE_BITS>> resp_snoop_data_o;
    sc_signal<sc_uint<DTAG_FL_TOTAL>> resp_snoop_flags_o;

    enum state_type {
        state_idle,
        state_ar,
        state_r,
        state_aw,
        state_w,
        state_b
    };

    struct RegistersType {
        sc_signal<sc_uint<3>> state;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_addr;
        sc_signal<bool> req_path;
        sc_signal<sc_uint<3>> req_cached;
        sc_signal<sc_biguint<L1CACHE_LINE_BITS>> req_wdata;
        sc_signal<sc_uint<L1CACHE_BYTES_PER_LINE>> req_wstrb;
        sc_signal<sc_biguint<3>> req_size;
        sc_signal<sc_biguint<3>> req_prot;
    } v, r;

    void R_RESET(RegistersType &iv) {
        iv.state = state_idle;
        iv.req_addr = 0;
        iv.req_path = 0;
        iv.req_cached = 0;
        iv.req_wdata = 0;
        iv.req_wstrb = 0;
        iv.req_size = 0;
        iv.req_prot = 0;
    }

    bool async_reset_;
};

}  // namespace debugger

#endif  // __DEBUGGER_RIVER_AMBA_H__
