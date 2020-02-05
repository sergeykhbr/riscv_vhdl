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
#include "river_top.h"
#include <systemc.h>

namespace debugger {

SC_MODULE(RiverAmba) {
    sc_in<bool> i_clk;                                  // CPU clock
    sc_in<bool> i_nrst;                                 // Reset: active LOW
    // AXI4 input structure:
    sc_in<bool> i_msti_aw_ready;
    sc_in<bool> i_msti_w_ready;
    sc_in<bool> i_msti_b_valid;
    sc_in<sc_uint<2>> i_msti_b_resp;
    sc_in<sc_uint<CFG_ID_BITS>> i_msti_b_id;
    sc_in<bool> i_msti_b_user;
    sc_in<bool> i_msti_ar_ready;
    sc_in<bool> i_msti_r_valid;
    sc_in<sc_uint<4>> i_msti_r_resp;                    // 0000=OKAY;0001=EXOKAY;0010=SLVERR;0011=DECER
                                                        // resp[2] PassDirty; rresp[3] IsShared
    sc_in<sc_biguint<L1CACHE_LINE_BITS>> i_msti_r_data;
    sc_in<bool> i_msti_r_last;
    sc_in<sc_uint<CFG_ID_BITS>> i_msti_r_id;
    sc_in<bool> i_msti_r_user;
    // AXI4 output structure:
    sc_out<bool> o_msto_aw_valid;
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_msto_aw_bits_addr;
    sc_out<sc_uint<8>> o_msto_aw_bits_len;              // burst len = len[7:0] + 1
    sc_out<sc_uint<3>> o_msto_aw_bits_size;             // 0=1B; 1=2B; 2=4B; 3=8B; ...
    sc_out<sc_uint<2>> o_msto_aw_bits_burst;            // 00=FIXED; 01=INCR; 10=WRAP; 11=reserved
    sc_out<bool> o_msto_aw_bits_lock;
    sc_out<sc_uint<4>> o_msto_aw_bits_cache;
    sc_out<sc_uint<3>> o_msto_aw_bits_prot;
    sc_out<sc_uint<4>> o_msto_aw_bits_qos;
    sc_out<sc_uint<4>> o_msto_aw_bits_region;
    sc_out<sc_uint<CFG_ID_BITS>> o_msto_aw_id;
    sc_out<bool> o_msto_aw_user;
    sc_out<bool> o_msto_w_valid;
    sc_out<sc_biguint<L1CACHE_LINE_BITS>> o_msto_w_data;
    sc_out<bool> o_msto_w_last;
    sc_out<sc_uint<DCACHE_BYTES_PER_LINE>> o_msto_w_strb;
    sc_out<bool> o_msto_w_user;
    sc_out<bool> o_msto_b_ready;
    sc_out<bool> o_msto_ar_valid;
    sc_out<sc_uint<BUS_ADDR_WIDTH>> o_msto_ar_bits_addr;
    sc_out<sc_uint<8>> o_msto_ar_bits_len;              // burst len = len[7:0] + 1
    sc_out<sc_uint<3>> o_msto_ar_bits_size;             // 0=1B; 1=2B; 2=4B; 3=8B; ...
    sc_out<sc_uint<2>> o_msto_ar_bits_burst;            // 00=FIXED; 01=INCR; 10=WRAP; 11=reserved
    sc_out<bool> o_msto_ar_bits_lock;
    sc_out<sc_uint<4>> o_msto_ar_bits_cache;
    sc_out<sc_uint<3>> o_msto_ar_bits_prot;
    sc_out<sc_uint<4>> o_msto_ar_bits_qos;
    sc_out<sc_uint<4>> o_msto_ar_bits_region;
    sc_out<sc_uint<CFG_ID_BITS>> o_msto_ar_id;
    sc_out<bool> o_msto_ar_user;
    sc_out<bool> o_msto_r_ready;
    // ACE signals:
    sc_in<bool> i_msti_ac_valid;
    sc_in<sc_uint<BUS_ADDR_WIDTH>> i_msti_ac_addr;
    sc_in<sc_uint<4>> i_msti_ac_snoop;                  // Table C3-19
    sc_in<sc_uint<3>> i_msti_ac_prot;
    sc_in<bool> i_msti_cr_ready;
    sc_in<bool> i_msti_cd_ready;
    sc_out<sc_uint<2>> o_msto_ar_domain;                // 00=Non-shareable (single master in domain)
    sc_out<sc_uint<4>> o_msto_ar_snoop;                 // Table C3-7:
    sc_out<sc_uint<2>> o_msto_ar_bar;                   // read barrier transaction
    sc_out<sc_uint<2>> o_msto_aw_domain;
    sc_out<sc_uint<4>> o_msto_aw_snoop;                 // Table C3-8
    sc_out<sc_uint<2>> o_msto_aw_bar;                   // write barrier transaction
    sc_out<bool> o_msto_ac_ready;
    sc_out<bool> o_msto_cr_valid;
    sc_out<sc_uint<5>> o_msto_cr_resp;
    sc_out<bool> o_msto_cd_valid;
    sc_out<sc_biguint<L1CACHE_LINE_BITS>> o_msto_cd_data;
    sc_out<bool> o_msto_cd_last;
    sc_out<bool> o_msto_rack;
    sc_out<bool> o_msto_wack;
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

    sc_signal<bool> req_mem_ready_i;
    sc_signal<bool> req_mem_path_o;
    sc_signal<bool> req_mem_valid_o;
    sc_signal<bool> req_mem_write_o;
    sc_signal<bool> req_mem_cached_o;
    sc_signal<sc_uint<BUS_ADDR_WIDTH>> req_mem_addr_o;
    sc_signal<sc_uint<L1CACHE_BYTES_PER_LINE>> req_mem_strob_o;
    sc_signal<sc_biguint<L1CACHE_LINE_BITS>> req_mem_data_o;
    sc_signal<bool> resp_mem_valid_i;
    sc_signal<bool> resp_mem_load_fault_i;
    sc_signal<bool> resp_mem_store_fault_i;

    enum state_type {
        idle,
        reading,
        writing
    };

    struct RegistersType {
        sc_signal<sc_uint<2>> state;
        sc_signal<bool> resp_path;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> w_addr;
        sc_signal<sc_uint<BUS_ADDR_WIDTH>> b_addr;
        sc_signal<bool> b_wait;
    } v, r;

    void R_RESET(RegistersType &iv) {
        iv.state = idle;
        iv.resp_path = 0;
        iv.w_addr = 0;
        iv.b_addr = 0;
        iv.b_wait = 0;
    }

    bool async_reset_;
};

}  // namespace debugger

#endif  // __DEBUGGER_RIVER_AMBA_H__
