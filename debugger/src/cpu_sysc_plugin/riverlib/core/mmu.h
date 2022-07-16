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
    sc_out<bool> o_fetch_page_fault;
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
    sc_in<bool> i_mmu_ena;                                  // MMU enabled in U and S modes. Sv48 only.
    sc_in<sc_uint<44>> i_mmu_ppn;                           // Physical Page Number from SATP CSR
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
    static const uint32_t CacheReq = 2;
    static const uint32_t WaitResp = 3;
    static const uint32_t HandleResp = 4;
    static const uint32_t UpdateTlb = 5;
    static const uint32_t AcceptFetch = 6;
    static const uint32_t FlushTlb = 7;
    static const int PTE_V = 0;
    static const int PTE_R = 1;
    static const int PTE_W = 2;
    static const int PTE_X = 3;
    static const int PTE_U = 4;
    static const int PTE_G = 5;
    static const int PTE_A = 6;
    static const int PTE_D = 7;

    struct Mmu_registers {
        sc_signal<sc_uint<3>> state;
        sc_signal<bool> req_x;
        sc_signal<bool> req_r;
        sc_signal<bool> req_w;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> req_pa;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> last_va;
        sc_signal<sc_uint<52>> last_pa;
        sc_signal<sc_uint<CFG_CPU_ADDR_BITS>> resp_addr;
        sc_signal<sc_uint<64>> resp_data;
        sc_signal<sc_uint<8>> pte_permission;               // See permission bits: DAGUXWRV
        sc_signal<bool> ex_load_fault;
        sc_signal<bool> ex_mpu_executable;
        sc_signal<bool> ex_page_fault;
        sc_signal<bool> tlb_hit;
        sc_signal<sc_uint<4>> tlb_level;
        sc_signal<sc_biguint<CFG_MMU_PTE_DWIDTH>> tlb_wdata;
        sc_signal<sc_uint<CFG_MMU_TLB_AWIDTH>> tlb_flush_cnt;
        sc_signal<sc_uint<CFG_MMU_TLB_AWIDTH>> tlb_flush_adr;
    } v, r;

    void Mmu_r_reset(Mmu_registers &iv) {
        iv.state = FlushTlb;
        iv.req_x = 0;
        iv.req_r = 0;
        iv.req_w = 0;
        iv.req_pa = 0;
        iv.last_va = ~0ull;
        iv.last_pa = ~0ull;
        iv.resp_addr = 0;
        iv.resp_data = 0;
        iv.pte_permission = 0;
        iv.ex_load_fault = 0;
        iv.ex_mpu_executable = 0;
        iv.ex_page_fault = 0;
        iv.tlb_hit = 0;
        iv.tlb_level = 0;
        iv.tlb_wdata = 0;
        iv.tlb_flush_cnt = ~0ul;
        iv.tlb_flush_adr = 0;
    }

    sc_signal<sc_uint<CFG_MMU_TLB_AWIDTH>> wb_tlb_adr;
    sc_signal<bool> w_tlb_wena;
    sc_signal<sc_biguint<CFG_MMU_PTE_DWIDTH>> wb_tlb_wdata;
    sc_signal<sc_biguint<CFG_MMU_PTE_DWIDTH>> wb_tlb_rdata;

    MmuTlb *tlb;

};

}  // namespace debugger

