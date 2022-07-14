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
#include "mmu_tlb.h"

namespace debugger {

SC_MODULE(Mmu) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_out<bool> o_fetch_req_ready;
    sc_in<bool> i_fetch_addr_valid;
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_fetch_addr;
    sc_out<bool> o_fetch_data_valid;
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_fetch_data_addr;
    sc_out<sc_uint<64>> o_fetch_data;
    sc_out<bool> o_fetch_load_fault;
    sc_out<bool> o_fetch_executable;
    sc_in<bool> i_fetch_resp_ready;
    sc_in<bool> i_mem_req_ready;
    sc_out<bool> o_mem_addr_valid;
    sc_out<sc_uint<CFG_CPU_ADDR_BITS>> o_mem_addr;
    sc_in<bool> i_mem_data_valid;
    sc_in<sc_uint<CFG_CPU_ADDR_BITS>> i_mem_data_addr;
    sc_in<sc_uint<64>> i_mem_data;
    sc_in<bool> i_mem_load_fault;
    sc_in<bool> i_mem_executable;
    sc_out<bool> o_mem_resp_ready;
    sc_in<sc_uint<2>> i_prv;                                // CPU priviledge level
    sc_in<sc_uint<RISCV_ARCH>> i_satp;                      // Supervisor Adress Translation and Protection
    sc_in<bool> i_flush_pipeline;                           // reset pipeline and cache

    void comb();
    void registers();

    SC_HAS_PROCESS(Mmu);

    Mmu(sc_module_name name,
        bool async_reset);
    virtual ~Mmu();

    void generateVCD(sc_trace_file *i_vcd, sc_trace_file *o_vcd);

 private:
    bool async_reset_;

    static const uint32_t Idle = 0;
    static const uint32_t CheckTlb = 1;
    static const uint32_t WaitReqAccept = 2;
    static const uint32_t WaitResp = 3;

    struct Mmu_registers {
        sc_signal<sc_uint<2>> state;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_va;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> last_va;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> last_pa;
        sc_signal<bool> req_valid;
        sc_signal<bool> resp_ready;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_addr;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> mem_resp_shadow;// the same as memory response but internal
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> pc;
        sc_signal<sc_uint<64>> instr;
        sc_signal<bool> instr_load_fault;
        sc_signal<bool> instr_executable;
    } v, r;

    void Mmu_r_reset(Mmu_registers &iv) {
        iv.state = Idle;
        iv.req_va = 0;
        iv.last_va = ~0ull;
        iv.last_pa = ~0ull;
        iv.req_valid = 0;
        iv.resp_ready = 0;
        iv.req_addr = ~0ull;
        iv.mem_resp_shadow = ~0ull;
        iv.pc = ~0ull;
        iv.instr = 0;
        iv.instr_load_fault = 0;
        iv.instr_executable = 0;
    }

    sc_signal<sc_uint<CFG_MMU_TLB_AWIDTH>> wb_tlb_adr;
    sc_signal<bool> w_tlb_wena;
    sc_signal<sc_biguint<CFG_MMU_PTE_DWIDTH>> wb_tlb_wdata;
    sc_signal<sc_biguint<CFG_MMU_PTE_DWIDTH>> wb_tlb_rdata;

    MmuTlb *tlb;

};

}  // namespace debugger

