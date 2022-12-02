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
#include "../../techmap/mem/ram_mmu_tech.h"

namespace debugger {

SC_MODULE(Mmu) {
 public:
    sc_in<bool> i_clk;                                      // CPU clock
    sc_in<bool> i_nrst;                                     // Reset: active LOW
    sc_out<bool> o_core_req_ready;
    sc_in<bool> i_core_req_valid;
    sc_in<sc_uint<RISCV_ARCH>> i_core_req_addr;
    sc_in<bool> i_core_req_fetch;                           // Memory request from 0=fetcher; 1=memaccess
    sc_in<sc_uint<MemopType_Total>> i_core_req_type;        // Memory operation type
    sc_in<sc_uint<64>> i_core_req_wdata;                    // Data path requested data (write transaction)
    sc_in<sc_uint<8>> i_core_req_wstrb;                     // 8-bytes aligned strobs
    sc_in<sc_uint<2>> i_core_req_size;                      // 1,2,4 or 8-bytes operation for uncached access
    sc_out<bool> o_core_resp_valid;
    sc_out<sc_uint<RISCV_ARCH>> o_core_resp_addr;
    sc_out<sc_uint<64>> o_core_resp_data;
    sc_out<bool> o_core_resp_load_fault;                    // Ex.2./Ex.5. Instruction access fault when = 0 and fetch or Load access fault
    sc_out<bool> o_core_resp_store_fault;                   // Ex.7. Store/AMO access fault
    sc_out<bool> o_core_resp_page_x_fault;                  // Ex.12 Instruction page fault
    sc_out<bool> o_core_resp_page_r_fault;                  // Ex.13 Load page fault
    sc_out<bool> o_core_resp_page_w_fault;                  // Ex.15 Store/AMO page fault
    sc_in<bool> i_core_resp_ready;
    sc_in<bool> i_mem_req_ready;
    sc_out<bool> o_mem_req_valid;
    sc_out<sc_uint<RISCV_ARCH>> o_mem_req_addr;
    sc_out<sc_uint<MemopType_Total>> o_mem_req_type;        // Memory operation type
    sc_out<sc_uint<64>> o_mem_req_wdata;                    // Data path requested data (write transaction)
    sc_out<sc_uint<8>> o_mem_req_wstrb;                     // 8-bytes aligned strobs
    sc_out<sc_uint<2>> o_mem_req_size;                      // 1,2,4 or 8-bytes operation for uncached access
    sc_in<bool> i_mem_resp_valid;
    sc_in<sc_uint<RISCV_ARCH>> i_mem_resp_addr;
    sc_in<sc_uint<64>> i_mem_resp_data;
    sc_in<bool> i_mem_resp_load_fault;
    sc_in<bool> i_mem_resp_store_fault;
    sc_out<bool> o_mem_resp_ready;
    sc_in<bool> i_mmu_ena;                                  // MMU enabled in U and S modes. Sv39 or Sv48 are implemented.
    sc_in<bool> i_mmu_sv39;                                 // MMU sv39 is active
    sc_in<bool> i_mmu_sv48;                                 // MMU sv48 is active
    sc_in<sc_uint<44>> i_mmu_ppn;                           // Physical Page Number from SATP CSR
    sc_in<bool> i_mprv;                                     // modify priviledge flag can be active in m-mode
    sc_in<bool> i_mxr;                                      // make executabale readable
    sc_in<bool> i_sum;                                      // permit Supervisor User Mode access
    sc_in<bool> i_fence;                                    // reset TBL entries at specific address
    sc_in<sc_uint<RISCV_ARCH>> i_fence_addr;                // Fence address: 0=clean all TBL

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
    static const uint32_t WaitRespNoMmu = 1;
    static const uint32_t WaitRespLast = 2;
    static const uint32_t CheckTlb = 3;
    static const uint32_t CacheReq = 4;
    static const uint32_t WaitResp = 5;
    static const uint32_t HandleResp = 6;
    static const uint32_t UpdateTlb = 7;
    static const uint32_t AcceptCore = 8;
    static const uint32_t FlushTlb = 9;
    static const int PTE_V = 0;
    static const int PTE_R = 1;
    static const int PTE_W = 2;
    static const int PTE_X = 3;
    static const int PTE_U = 4;
    static const int PTE_G = 5;
    static const int PTE_A = 6;
    static const int PTE_D = 7;

    struct Mmu_registers {
        sc_signal<sc_uint<4>> state;
        sc_signal<bool> req_x;
        sc_signal<bool> req_r;
        sc_signal<bool> req_w;
        sc_signal<sc_uint<RISCV_ARCH>> req_pa;
        sc_signal<sc_uint<MemopType_Total>> req_type;
        sc_signal<sc_uint<64>> req_wdata;
        sc_signal<sc_uint<8>> req_wstrb;
        sc_signal<sc_uint<2>> req_size;
        sc_signal<bool> req_flush;
        sc_signal<bool> last_mmu_ena;
        sc_signal<sc_uint<RISCV_ARCH>> last_va;
        sc_signal<sc_uint<52>> last_pa;
        sc_signal<sc_uint<8>> last_permission;              // Last permisison flags: DAGUXWRV
        sc_signal<sc_uint<2>> last_page_size;
        sc_signal<sc_uint<RISCV_ARCH>> resp_addr;
        sc_signal<sc_uint<64>> resp_data;
        sc_signal<bool> resp_load_fault;
        sc_signal<bool> resp_store_fault;
        sc_signal<bool> ex_page_fault;
        sc_signal<bool> tlb_hit;
        sc_signal<sc_uint<4>> tlb_level;
        sc_signal<sc_uint<2>> tlb_page_size;
        sc_signal<sc_biguint<CFG_MMU_PTE_DWIDTH>> tlb_wdata;
        sc_signal<sc_uint<CFG_MMU_TLB_AWIDTH>> tlb_flush_cnt;
        sc_signal<sc_uint<CFG_MMU_TLB_AWIDTH>> tlb_flush_adr;
    } v, r;

    void Mmu_r_reset(Mmu_registers &iv) {
        iv.state = FlushTlb;
        iv.req_x = 0;
        iv.req_r = 0;
        iv.req_w = 0;
        iv.req_pa = 0ull;
        iv.req_type = 0;
        iv.req_wdata = 0ull;
        iv.req_wstrb = 0;
        iv.req_size = 0;
        iv.req_flush = 0;
        iv.last_mmu_ena = 0;
        iv.last_va = ~0ull;
        iv.last_pa = ~0ull;
        iv.last_permission = 0;
        iv.last_page_size = 0;
        iv.resp_addr = 0ull;
        iv.resp_data = 0ull;
        iv.resp_load_fault = 0;
        iv.resp_store_fault = 0;
        iv.ex_page_fault = 0;
        iv.tlb_hit = 0;
        iv.tlb_level = 0;
        iv.tlb_page_size = 0;
        iv.tlb_wdata = 0ull;
        iv.tlb_flush_cnt = ~0ul;
        iv.tlb_flush_adr = 0;
    }

    sc_signal<sc_uint<CFG_MMU_TLB_AWIDTH>> wb_tlb_adr;
    sc_signal<bool> w_tlb_wena;
    sc_signal<sc_biguint<CFG_MMU_PTE_DWIDTH>> wb_tlb_wdata;
    sc_signal<sc_biguint<CFG_MMU_PTE_DWIDTH>> wb_tlb_rdata;

    ram_mmu_tech<CFG_MMU_TLB_AWIDTH, CFG_MMU_PTE_DWIDTH> *tlb;

};

}  // namespace debugger

